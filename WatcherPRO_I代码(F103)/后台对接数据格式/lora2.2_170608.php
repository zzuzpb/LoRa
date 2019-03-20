<?php
/**
 * 增加长度偶数判断，增加版本错误返回判断，增加数据域溢出判断
 * 更正叶面温度数据类型为有符号一位小数
 * 
 **/
function decodeData($str)
{
	$len = strlen($str);
	if ($len<36||$len%2!=0){
		return null;
	}
	sscanf($str,"%02x",$ver);
	$values = array();
	if ($ver!=2) {
		return null;
	}
	$count = hexdec($str[2].$str[3]);
	if ($len!=$count*4+16||$len<5) {
		return null;
	}
	for ($i = 0; $i < $count; $i++) {
		 $nums[$i] = hexdec($str[16+$i*4+2].$str[16+$i*4+3].$str[16+$i*4].$str[16+$i*4+1]);
	}
	$values["Battery"] = $nums[0];
	$values["Illumination"] = ($nums[1]<<16|$nums[2])/100.0;
	if ($nums[3]>0x8000) {
		$values["AirTemperature"] = (-(~($nums[3]-1)&0xffff))/10.0;
	}
	else {
		$values["AirTemperature"] = $nums[3]/10.0;
	}
	$values["AirHumidity"] = $nums[4]/10.0;
	$sockets = array("Socket0","Socket1","Socket2","Socket3","Socket4","Socket5");
	$index = 5;
	for ($i=0; $i <=5; $i++){
		$tag = hexdec($str[$i*2+4].$str[$i*2+5]);
		$fieldType = ($tag&0xf8)>>3;
		$fieldCount= $tag&0x07;
		$fieldOffset = 0;
		if ($fieldCount==0) {
			continue;
		}
		else if ($fieldCount+$index>$count){
		    return null;
		}
		$s = array();
		do {
			switch ($fieldType+$fieldOffset) {
				case 0:
					$s["Battery"]=$nums[$index++];
					break;
				case 1:
					$s["Illumination"]=($nums[$index++]<<16|$nums[$index++])/100.0;
					break;
				case 2:
					if ($nums[$index]>0x8000) {
						$s["AirTemperature"] = (-(~($nums[$index++]-1)&0xffff))/10.0;
					}
					else {
						$s["AirTemperature"] = $nums[$index++]/10.0;
					}
					break;
				case 3:
					$s["AirHumidity"] = $nums[$index++]/10.0;
					break;
				case 4:
					if ($nums[$index]>0x8000) {
						$s["SoilTemperature"] = (-(~($nums[$index++]-1)&0xffff))/10.0;
					}
					else {
						$s["SoilTemperature"] = $nums[$index++]/10.0;
					}
					break;
				case 5:
					$s["SoilHumidity"] = $nums[$index++]/10.0;
					break;
				case 6:
					$s["Photosynthetic"]=$nums[$index++];
					break;
				case 7:
					if ($nums[$index]>0x8000) {
						$s["LeafTemperature"] = -(~($nums[$index++]-1)&0xffff)/10.0;
					}
					else {
						$s["LeafTemperature"] = $nums[$index++]/10.0;
					}
					break;
				case 8:
					$s["LeafHumidity"]=$nums[$index++]/100.0;
					break;
				case 9:
					$s["RainFall"]=$nums[$index++]/10.0;
					break;
				case 10:
					$s["WindSpeed"]=$nums[$index++]/10.0;
					break;
				case 11:
					$s["WindDirection"]=$nums[$index++]/10.0;
					break;
				case 12:
					$s["SoilEleCond"]=$nums[$index++]/100.0;
					break;
				case 13:
					$s["SoilSalinity"]=$nums[$index++]/10.0;
					break;
				case 14:
					$s["Co2"]=$nums[$index++];
					break;
				case 15:
					$s["Atmospheric"]=$nums[$index++]/10.0;
					break;
				case 16:
					$s["SoilPH"]=$nums[$index++]/100.0;
					break;
				default:
					continue;
			}
		}while(++$fieldOffset<$fieldCount&&$index<$count);
		$values[$sockets[$i]]=$s;
	}
	return $values;
}

$str = "0205000000000000430000000000e5feff01";

$d = decodeData($str);
var_dump($d);

?>