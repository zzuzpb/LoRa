#include "NBI_gps.h"
NBI_Gpsdata gpsdata = {HAL_ERROR,HAL_ERROR};
void ansyGpsData(char *line)
{
    switch (minmea_sentence_id(line, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (minmea_parse_rmc(&frame, line)) 
            {
//                printf("$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
//                        frame.latitude.value, frame.latitude.scale,
//                        frame.longitude.value, frame.longitude.scale,
//                        frame.speed.value, frame.speed.scale);
//                printf("$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
//                        minmea_rescale(&frame.latitude, 1000),
//                        minmea_rescale(&frame.longitude, 1000),
//                        minmea_rescale(&frame.speed, 1000));
//                printf("$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
//                        minmea_tocoord(&frame.latitude),
//                        minmea_tocoord(&frame.longitude),
//                        minmea_tofloat(&frame.speed));							 
							
							
//							 printf("latitude = %d\r\n",minmea_rescale(&frame.latitude, 1000));
							 if(minmea_rescale(&frame.latitude, 1000) != 0)
							 {
								 //printf("lat = %d\r\n",minmea_rescale(&frame.latitude, 1000));
								 gpsdata.fix = HAL_OK;
							 }
							 else
							 {								 
								 gpsdata.fix = HAL_ERROR;
							 }							
							 gpsdata.islocal = HAL_OK;
							 if(gpsdata.fix == HAL_OK)
							 {
								 gpsdata.latitude = minmea_tocoord(&frame.latitude);    
								 gpsdata.longitude = minmea_tocoord(&frame.longitude);

								 gpsdata.day = frame.date.day;
								 gpsdata.year =frame.date.year;
								 gpsdata.month = frame.date.month;
								 gpsdata.hours = frame.time.hours;
								 gpsdata.seconds = frame.time.seconds;
								 gpsdata.minutes = frame.time.minutes;
								 gpsdata.microseconds = frame.time.microseconds;
							 }
            }
            else {
//                printf("$xxRMC sentence is not parsed\n");
//								gpsdata.islocal = HAL_ERROR; 
									
            }
        } break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (minmea_parse_gga(&frame, line)) {               
            }
            else {
                //printf("$xxGGA sentence is not parsed\n");
            }
        } break;

        case MINMEA_SENTENCE_GST: {
//            struct minmea_sentence_gst frame;
//            if (minmea_parse_gst(&frame, line)) {
//                printf("$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n",
//                        frame.latitude_error_deviation.value, frame.latitude_error_deviation.scale,
//                        frame.longitude_error_deviation.value, frame.longitude_error_deviation.scale,
//                        frame.altitude_error_deviation.value, frame.altitude_error_deviation.scale);
//                printf("$xxGST fixed point latitude,longitude and altitude error deviation"
//                       " scaled to one decimal place: (%d,%d,%d)\n",
//                        minmea_rescale(&frame.latitude_error_deviation, 10),
//                        minmea_rescale(&frame.longitude_error_deviation, 10),
//                        minmea_rescale(&frame.altitude_error_deviation, 10));
//                printf("$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
//                        minmea_tofloat(&frame.latitude_error_deviation),
//                        minmea_tofloat(&frame.longitude_error_deviation),
//                        minmea_tofloat(&frame.altitude_error_deviation));
//            }
//            else {
//                printf("$xxGST sentence is not parsed\n");
//            }
        } break;

        case MINMEA_SENTENCE_GSV: {
//            struct minmea_sentence_gsv frame;
//            if (minmea_parse_gsv(&frame, line)) {
//                printf("$xxGSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
//                printf("$xxGSV: sattelites in view: %d\n", frame.total_sats);
//                for (int i = 0; i < 4; i++)
//                    printf("$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
//                        frame.sats[i].nr,
//                        frame.sats[i].elevation,
//                        frame.sats[i].azimuth,
//                        frame.sats[i].snr);
//            }
//            else {
//                printf("$xxGSV sentence is not parsed\n");
//            }
        } break;

        case MINMEA_SENTENCE_VTG: {
//           struct minmea_sentence_vtg frame;
//           if (minmea_parse_vtg(&frame, line)) {
//                printf("$xxVTG: true track degrees = %f\n",
//                       minmea_tofloat(&frame.true_track_degrees));
//                printf("        magnetic track degrees = %f\n",
//                       minmea_tofloat(&frame.magnetic_track_degrees));
//                printf("        speed knots = %f\n",
//                        minmea_tofloat(&frame.speed_knots));
//                printf("        speed kph = %f\n",
//                        minmea_tofloat(&frame.speed_kph));
//           }
//           else {
//                printf("$xxVTG sentence is not parsed\n");
//           }
        } break;

        case MINMEA_SENTENCE_ZDA: {
//            struct minmea_sentence_zda frame;
//            if (minmea_parse_zda(&frame, line)) {
//                printf("$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
//                       frame.time.hours,
//                       frame.time.minutes,
//                       frame.time.seconds,
//                       frame.date.day,
//                       frame.date.month,
//                       frame.date.year,
//                       frame.hour_offset,
//                       frame.minute_offset);
//            }
//            else {
//                printf("$xxZDA sentence is not parsed\n");
//            }
        } break;

        case MINMEA_INVALID: {
//            printf("$xxxxx sentence is not valid\n");
        } break;

        default: {
//            printf("$xxxxx sentence is not parsed\n");
        } break;
    }

}

void gps_input()
{
	static char buff[MINMEA_MAX_LENGTH] = {0};
	static int index = 0;
	uint8_t ch;
	while(FIFO_UartReadByte(&usart_gps,&ch) == HAL_OK)
	{				
		buff[index++] = ch;		
		if(ch == '\n')
		{		
			ansyGpsData(buff);
			memset(buff,0,index);
			index = 0;		
		}
	}
}




//判断是否能够从GPS中读到数据
HAL_StatusTypeDef getGpsStatus()
{
	uint8_t ch;
	setCountTime(10);
	while(1)
	{
		if(FIFO_UartReadByte(&usart_gps,&ch) == HAL_ERROR)
		{
			if(GetCountTIme() == 0)
				return HAL_ERROR;
		}
		else
			return HAL_OK;
	}
}





















