#ifndef __AP_GPS_NMEA_H__
#define __AP_GPS_NMEA_H__

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#include "../AP_UartDeriver/uartderiver.hpp"
extern "C"{
//必须声明为C文件才可链接到C++文件中
#include "../../nuttx/configs/stm32f4discovery/src/stm32_drv_hrt.h"
}


class UARTDriver;
/// NMEA parser
///
class AP_GPS_NMEA {
public:
	
	void GPSInit(void) ;

    /// Checks the serial receive buffer for characters,
    /// attempts to parse NMEA data and updates internal state
    /// accordingly.

	
    bool read(UARTDriver * uart, uint16_t * num_last);

	struct NMEA_detect_state {
	  uint8_t step;
	  uint8_t ck;
	  };
	NMEA_detect_state nmea_detect_state;

	bool detect(struct NMEA_detect_state &state, uint8_t data);

	bool gps_good_or_bad(void){return _gps_data_good;};

//添加
	

    /// GPS status codes
    enum GPS_Status {
        NO_GPS = 0,             ///< No GPS connected/detected
        NO_FIX = 1,             ///< Receiving valid GPS messages but no lock
        GPS_OK_FIX_2D = 2,      ///< Receiving valid messages and 2D lock
        GPS_OK_FIX_3D = 3,      ///< Receiving valid messages and 3D lock
        GPS_OK_FIX_3D_DGPS = 4, ///< Receiving valid messages and 3D lock with differential improvements
        GPS_OK_FIX_3D_RTK = 5,  ///< Receiving valid messages and 3D lock, with relative-positioning improvements
    };

	struct  Location {
		// by making alt 24 bit we can make p1 in a command 16 bit,
		// allowing an accurate angle in centi-degrees. This keeps the
		// storage cost per mission item at 15 bytes, and allows mission
		// altitudes of up to +/- 83km
		int32_t alt; 									///< param 2 - Altitude in centimeters (meters * 100)
		int32_t lat;										///< param 3 - Latitude * 10**7
		int32_t lng;										///< param 4 - Longitude * 10**7
	};


	/*
	  The GPS_State structure is filled in by the backend driver as it
	  parses each message from the GPS.
	 */
	struct GPS_State {
	uint8_t instance; // the instance number of this GPS

	// all the following fields must all be filled by the backend driver
	GPS_Status status;					///< driver fix status
	uint32_t time_week_ms;				///< GPS time (milliseconds from start of GPS week)
	uint16_t time_week; 				///< GPS week number
	uint32_t utc_date;				///< GPS time (milliseconds from start of GPS week)
	uint32_t utc_ms; 				///< GPS week number
	Location location;					///< last fix location
	float ground_speed; 				///< ground speed in m/sec
	float ground_course;				///< ground course in degrees
	uint16_t hdop;						///< horizontal dilution of precision in cm
	uint16_t vdop;						///< vertical dilution of precision in cm
	uint8_t num_sats;					///< Number of visible satellites		 
	float speed_accuracy;
	float horizontal_accuracy;
	float vertical_accuracy;
	bool have_vertical_velocity:1;		///< does this GPS give vertical velocity?
	bool have_speed_accuracy:1;
	bool have_horizontal_accuracy:1;
	bool have_vertical_accuracy:1;
	uint32_t last_gps_time_ms;			///< the system time we got the last GPS timestamp, milliseconds
};

      GPS_State state;           ///< public state for this instance




/*
   fill in time_week_ms and time_week from BCD date and time components
   assumes MTK19 millisecond form of bcd_time
*/
void     make_gps_time(uint32_t bcd_date, uint32_t bcd_milliseconds);
uint64_t time_epoch_usec(void);
void     set_system_clock(uint64_t time_utc_usec);


	

private:
    /// Coding for the GPS sentences that the parser handles
    enum _sentence_types {      //there are some more than 10 fields in some sentences , thus we have to increase these value.
        _GPS_SENTENCE_RMC = 32,
        _GPS_SENTENCE_GGA = 64,
        _GPS_SENTENCE_VTG = 96,
        _GPS_SENTENCE_OTHER = 0
    };

    /// Update the decode state machine with a new character
    ///
    /// @param	c		The next character in the NMEA input stream
    /// @returns		True if processing the character has resulted in
    ///					an update to the GPS state
    ///
    bool                        _decode(char c);

    /// Return the numeric value of an ascii hex character
    ///
    /// @param	a		The character to be converted
    /// @returns		The value of the character as a hex digit
    ///
    int16_t                     _from_hex(char a);

    /// Parses the @p as a NMEA-style decimal number with
    /// up to 3 decimal digits.
    ///
    /// @returns		The value expressed by the string in @p,
    ///					multiplied by 100.
    ///
    static int32_t _parse_decimal_100(const char *p);

    /// Parses the current term as a NMEA-style degrees + minutes
    /// value with up to four decimal digits.
    ///
    /// This gives a theoretical resolution limit of around 1cm.
    ///
    /// @returns		The value expressed by the string in _term,
    ///					multiplied by 1e7.
    ///
    uint32_t    _parse_degrees();

    /// Processes the current term when it has been deemed to be
    /// complete.
    ///
    /// Each GPS message is broken up into terms separated by commas.
    /// Each term is then processed by this function as it is received.
    ///
    /// @returns		True if completing the term has resulted in
    ///					an update to the GPS state.
    bool                        _term_complete();

    /// return true if we have a new set of NMEA messages
    bool _have_new_message(void);

    uint8_t _parity;                                                    ///< NMEA message checksum accumulator
    bool _is_checksum_term;                                     ///< current term is the checksum
    char _term[15];                                                     ///< buffer for the current term within the current sentence
    uint8_t _sentence_type;                                     ///< the sentence type currently being processed
    uint8_t _term_number;                                       ///< term index within the current sentence
    uint8_t _term_offset;                                       ///< character offset with the term being received
    bool _gps_data_good;                                        ///< set when the sentence indicates data is good

    // The result of parsing terms within a message is stored temporarily until
    // the message is completely processed and the checksum validated.
    // This avoids the need to buffer the entire message.
    int32_t _new_time;                                                  ///< time parsed from a term
    int32_t _new_date;                                                  ///< date parsed from a term
    int32_t _new_latitude;                                      ///< latitude parsed from a term
    int32_t _new_longitude;                                     ///< longitude parsed from a term
    int32_t _new_altitude;                                      ///< altitude parsed from a term
    int32_t _new_speed;                                                 ///< speed parsed from a term
    int32_t _new_course;                                        ///< course parsed from a term
    uint16_t _new_hdop;                                                 ///< HDOP parsed from a term
    uint8_t _new_satellite_count;                       ///< satellite count parsed from a term

    uint32_t _last_RMC_ms = 0;
    uint32_t _last_GGA_ms = 0;
    uint32_t _last_VTG_ms = 0;
	
};

#endif
