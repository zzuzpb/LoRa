#include "AP_GPS_NMEA.hpp"

// optionally log all NMEA data for debug purposes
// #define NMEA_LOG_PATH "nmea.log"

#ifdef NMEA_LOG_PATH
#include <stdio.h>
#endif

// Convenience macros //////////////////////////////////////////////////////////
//
#define DIGIT_TO_VAL(_x)        (_x - '0')
#define hexdigit(x) ((x)>9?'A'+((x)-10):'0'+(x))

void AP_GPS_NMEA::GPSInit(void)		
{
	_parity = 0;
	_is_checksum_term = false;
	_sentence_type = 0;
	_term_number = 0;
	_term_offset = 0;
	_gps_data_good = false;
	memset(_term, 0, sizeof(_term));
}


bool AP_GPS_NMEA::read(UARTDriver * uart ,uint16_t* num_last)
{
    uint16_t numc;
    bool parsed = false;

    numc = uart->available();
	*num_last  = numc;

    while (numc--) {
        char c = uart->read();
#ifdef NMEA_LOG_PATH
        static FILE *logf = NULL;
        if (logf == NULL) {
            logf = fopen(NMEA_LOG_PATH, "wb");
        }
        if (logf != NULL) {
            ::fwrite(&c, 1, 1, logf);
        }
#endif
        if (_decode(c)) {
            parsed = true;
        }
    }
	
    return parsed;
}

bool AP_GPS_NMEA::_decode(char c)
{
    bool valid_sentence = false;

    switch (c) {
    case ',': // term terminators
        _parity ^= c;
        /* no break */
    case '\r':
    case '\n':
    case '*':
        if (_term_offset < sizeof(_term)) {
            _term[_term_offset] = 0;
            valid_sentence = _term_complete();
        }
        ++_term_number;
        _term_offset = 0;
        _is_checksum_term = c == '*';
        return valid_sentence;

    case '$': // sentence begin
        _term_number = _term_offset = 0;
        _parity = 0;
        _sentence_type = _GPS_SENTENCE_OTHER;
        _is_checksum_term = false;
        _gps_data_good = false;
        return valid_sentence;
    }

    // ordinary characters
    if (_term_offset < sizeof(_term) - 1)
        _term[_term_offset++] = c;
    if (!_is_checksum_term)
        _parity ^= c;

    return valid_sentence;
}

//
// internal utilities
//
int16_t AP_GPS_NMEA::_from_hex(char a)
{
    if (a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    else if (a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    else
        return a - '0';
}

int32_t AP_GPS_NMEA::_parse_decimal_100(const char *p)
{
    char *endptr = nullptr;
    long ret = 100 * strtol(p, &endptr, 10);
    int sign = ret < 0 ? -1 : 1;

    if (ret >= (long)INT32_MAX) {
        return INT32_MAX;
    }
    if (ret <= (long)INT32_MIN) {
        return INT32_MIN;
    }
    if (endptr == nullptr || *endptr != '.') {
        return ret;
    }

    if (isdigit(endptr[1])) {
        ret += sign * 10 * DIGIT_TO_VAL(endptr[1]);
        if (isdigit(endptr[2])) {
            ret += sign * DIGIT_TO_VAL(endptr[2]);
            if (isdigit(endptr[3])) {
                ret += sign * (DIGIT_TO_VAL(endptr[3]) >= 5);
            }
        }
    }
    return ret;
}

/*
  parse a NMEA latitude/longitude degree value. The result is in degrees*1e7
 */
uint32_t AP_GPS_NMEA::_parse_degrees()
{
    char *p, *q;
    uint8_t deg = 0, min = 0;
    float frac_min = 0;
    int32_t ret = 0;

    // scan for decimal point or end of field
    for (p = _term; *p && isdigit(*p); p++)
        ;
    q = _term;

    // convert degrees
    while ((p - q) > 2 && *q) {
        if (deg)
            deg *= 10;
        deg += DIGIT_TO_VAL(*q++);
    }

    // convert minutes
    while (p > q && *q) {
        if (min)
            min *= 10;
        min += DIGIT_TO_VAL(*q++);
    }

    // convert fractional minutes
    if (*p == '.') {
        q = p + 1;
        float frac_scale = 0.1f;
        while (*q && isdigit(*q)) {
            frac_min += DIGIT_TO_VAL(*q) * frac_scale;
            q++;
            frac_scale *= 0.1f;
        }
    }
    ret = (deg * (int32_t)10000000UL);
    ret += (min * (int32_t)10000000UL / 60);
    ret += (int32_t) (frac_min * (1.0e7f / 60.0f));
    return ret;
}

/*
  see if we have a new set of NMEA messages
 */
bool AP_GPS_NMEA::_have_new_message()
{
    if (_last_RMC_ms == 0 ||
        _last_GGA_ms == 0) {
        return false;
    }
    uint32_t now = hrt_absolute_time();
    if (now - _last_RMC_ms > 150 ||
        now - _last_GGA_ms > 150) {
        return false;
    }
    if (_last_VTG_ms != 0 && 
        now - _last_VTG_ms > 150) {
        return false;
    }
    // prevent these messages being used again
    if (_last_VTG_ms != 0) {
        _last_VTG_ms = 1;
    }
    _last_GGA_ms = 1;
    _last_RMC_ms = 1;
    return true;
}

// Processes a just-completed term
// Returns true if new sentence has just passed checksum test and is validated
bool AP_GPS_NMEA::_term_complete()
{
    // handle the last term in a message
    if (_is_checksum_term) {
        uint8_t checksum = 16 * _from_hex(_term[0]) + _from_hex(_term[1]);
        if (checksum == _parity) {
            if (_gps_data_good) {
                uint32_t now = hrt_absolute_time();
                switch (_sentence_type) {
                case _GPS_SENTENCE_RMC:
                    _last_RMC_ms = now;
                    state.location.lat     = _new_latitude;
                    state.location.lng     = _new_longitude;
                    state.ground_speed     = _new_speed*0.01f;
                    make_gps_time(_new_date, _new_time * 10);
                    state.last_gps_time_ms = now;
                    // To-Do: add support for proper reporting of 2D and 3D fix
                    state.status           = GPS_OK_FIX_3D;
                    break;
                case _GPS_SENTENCE_GGA:
                    _last_GGA_ms = now;
                    state.location.alt  = _new_altitude;
                    state.location.lat  = _new_latitude;
                    state.location.lng  = _new_longitude;
                    state.num_sats      = _new_satellite_count;
                    state.hdop          = _new_hdop;
                    // To-Do: add support for proper reporting of 2D and 3D fix
                    state.status        = GPS_OK_FIX_3D;
                    break;
                case _GPS_SENTENCE_VTG:
                    _last_VTG_ms = now;
                    state.ground_speed  = _new_speed*0.01f;
                    // VTG has no fix indicator, can't change fix status
                    break;
                }
            } else {
                switch (_sentence_type) {
                case _GPS_SENTENCE_RMC:
                case _GPS_SENTENCE_GGA:
                    // Only these sentences give us information about
                    // fix status.
                    state.status = NO_FIX;
                }
            }
            // see if we got a good message
            return _have_new_message();
        }
        // we got a bad message, ignore it
        return false;
    }

    // the first term determines the sentence type
    if (_term_number == 0) {
        /*
          The first two letters of the NMEA term are the talker
          ID. The most common is 'GP' but there are a bunch of others
          that are valid. We accept any two characters here.
         */
        if (_term[0] < 'A' || _term[0] > 'Z' ||
            _term[1] < 'A' || _term[1] > 'Z') {
            _sentence_type = _GPS_SENTENCE_OTHER;
            return false;
        }
        const char *term_type = &_term[2];
        if (strcmp(term_type, "RMC") == 0) {
            _sentence_type = _GPS_SENTENCE_RMC;
        } else if (strcmp(term_type, "GGA") == 0) {
            _sentence_type = _GPS_SENTENCE_GGA;
        } else if (strcmp(term_type, "VTG") == 0) {
            _sentence_type = _GPS_SENTENCE_VTG;
            // VTG may not contain a data qualifier, presume the solution is good
            // unless it tells us otherwise.
            _gps_data_good = true;
        } else {
            _sentence_type = _GPS_SENTENCE_OTHER;
        }
        return false;
    }

    // 32 = RMC, 64 = GGA, 96 = VTG
    if (_sentence_type != _GPS_SENTENCE_OTHER && _term[0]) {
        switch (_sentence_type + _term_number) {
        // operational status
        //
        case _GPS_SENTENCE_RMC + 2: // validity (RMC)
            _gps_data_good = _term[0] == 'A';
            break;
        case _GPS_SENTENCE_GGA + 6: // Fix data (GGA)
            _gps_data_good = _term[0] > '0';
            break;
        case _GPS_SENTENCE_VTG + 9: // validity (VTG) (we may not see this field)
            _gps_data_good = _term[0] != 'N';
            break;
        case _GPS_SENTENCE_GGA + 7: // satellite count (GGA)
            _new_satellite_count = atol(_term);
            break;
        case _GPS_SENTENCE_GGA + 8: // HDOP (GGA)
            _new_hdop = (uint16_t)_parse_decimal_100(_term);
            break;

        // time and date
        //
        case _GPS_SENTENCE_RMC + 1: // Time (RMC)
        case _GPS_SENTENCE_GGA + 1: // Time (GGA)
            _new_time = _parse_decimal_100(_term);
            break;
        case _GPS_SENTENCE_RMC + 9: // Date (GPRMC)
            _new_date = atol(_term);
            break;

        // location
        //
        case _GPS_SENTENCE_RMC + 3: // Latitude
        case _GPS_SENTENCE_GGA + 2:
            _new_latitude = _parse_degrees();
            break;
        case _GPS_SENTENCE_RMC + 4: // N/S
        case _GPS_SENTENCE_GGA + 3:
            if (_term[0] == 'S')
                _new_latitude = -_new_latitude;
            break;
        case _GPS_SENTENCE_RMC + 5: // Longitude
        case _GPS_SENTENCE_GGA + 4:
            _new_longitude = _parse_degrees();
            break;
        case _GPS_SENTENCE_RMC + 6: // E/W
        case _GPS_SENTENCE_GGA + 5:
            if (_term[0] == 'W')
                _new_longitude = -_new_longitude;
            break;
        case _GPS_SENTENCE_GGA + 9: // Altitude (GPGGA)
            _new_altitude = _parse_decimal_100(_term);
            break;

        // course and speed
        //
        case _GPS_SENTENCE_RMC + 7: // Speed (GPRMC)
        case _GPS_SENTENCE_VTG + 5: // Speed (VTG)
            _new_speed = (_parse_decimal_100(_term) * 514) / 1000;       // knots-> m/sec, approximiates * 0.514
            break;
        case _GPS_SENTENCE_RMC + 8: // Course (GPRMC)
        case _GPS_SENTENCE_VTG + 1: // Course (VTG)
            _new_course = _parse_decimal_100(_term);
            break;
        }
    }

    return false;
}

/*
  detect a NMEA GPS. Adds one byte, and returns true if the stream
  matches a NMEA string
 */
bool AP_GPS_NMEA::detect(struct NMEA_detect_state &detect_state, uint8_t data)
{
	switch (detect_state.step) {
	case 0:
		detect_state.ck = 0;
		if ('$' == data) {
			detect_state.step++;
		}
		break;
	case 1:
		if ('*' == data) {
			detect_state.step++;
		} else {
			detect_state.ck ^= data;
		}
		break;
	case 2:
		if (hexdigit(detect_state.ck>>4) == data) {
			detect_state.step++;
		} else {
			detect_state.step = 0;
		}
		break;
	case 3:
		if (hexdigit(detect_state.ck&0xF) == data) {
            detect_state.step = 0;
			return true;
		}
		detect_state.step = 0;
		break;
    }
    return false;
}


/**
   fill in time_week_ms and time_week from BCD date and time components
   assumes MTK19 millisecond form of bcd_time
 */
void AP_GPS_NMEA::make_gps_time(uint32_t bcd_date, uint32_t bcd_milliseconds)
{
    uint8_t year, mon, day, hour, min, sec;
    uint16_t msec;
	static bool set_date = false;

	state.utc_date = bcd_date;
	state.utc_ms   = bcd_milliseconds;

    year = bcd_date % 100;
    mon  = (bcd_date / 100) % 100;
    day  = bcd_date / 10000;

    uint32_t v = bcd_milliseconds;
    msec = v % 1000; v /= 1000;
    sec  = v % 100; v /= 100;
    min  = v % 100; v /= 100;
    hour = v % 100; v /= 100;

    int8_t rmon = mon - 2;
    if (0 >= rmon) {    
        rmon += 12;
        year -= 1;
    }

    // get time in seconds since unix epoch
    uint32_t ret = (year/4) - 15 + 367*rmon/12 + day;
    ret += year*365 + 10501;
    ret = ret*24 + hour;
    ret = ret*60 + min;
    ret = ret*60 + sec;

	

    // convert to time since GPS epoch
    ret -= 272764785UL;

    // get GPS week and time
    state.time_week = ret / (7*86400UL);
    state.time_week_ms = (ret % (7*86400UL)) * 1000;
    state.time_week_ms += msec;

	if(!set_date)
	{
		set_date = true;
		set_system_clock(time_epoch_usec());//…Ë÷√ ±º‰
	}
}


uint64_t AP_GPS_NMEA::time_epoch_usec(void)
{
    const uint64_t ms_per_week = 7000ULL*86400ULL;
    const uint64_t unix_offset = 17000ULL*86400ULL + 52*10*7000ULL*86400ULL - 15000ULL;
    uint64_t fix_time_ms = unix_offset + state.time_week*ms_per_week + state.time_week_ms;
    // add in the milliseconds since the last fix
    return (28800000ULL+fix_time_ms) * 1000ULL;
}

void AP_GPS_NMEA::set_system_clock(uint64_t time_utc_usec)
{
    timespec ts;
    ts.tv_sec = time_utc_usec/1.0e6f;
    ts.tv_nsec = (time_utc_usec % 1000000) * 1000;
    clock_settime(CLOCK_REALTIME, &ts);    
}






