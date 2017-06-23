##Functionnal Diagram of HidnSeek firmware
![Functionnal Diagram](HidnSeek-dia.png)

##php example code to decod SIGFOX Payload

```php
/**************************************************************************************************
HidnSeek 3_28 php - Copyright of the author Stephane Driussi - 23/03/2015

These php code may be used free of charge for personal, recreational and educational purposes only.

This program, or parts of it, may not be used for or in connection with any commercial purpose
without the explicit permission of the author Stephane Driussi.

The programs are supplied as is, it is up to individual to decide if the programs are suitable for
the intended purpose and free from errors.
**************************************************************************************************/

function hexTo32Float($strHex) {
  $binarydata32 = pack('H*',$strHex);
  $float32 = unpack("f", $binarydata32);
  return $float32[1];
}

function hexTo32Integer($strHex) {
  $binarydata32 = pack('H*',$strHex);
  $integer32 = unpack("V", $binarydata32);
  return $integer32[1];
}

$array = array();

// mlat and mlon are latitude and longitude GPS coordonates
$array['mlat']=hexTo32Float(substr(htmlspecialchars($_REQUEST["Payload"]),0,8));
$array['mlon']=hexTo32Float(substr(htmlspecialchars($_REQUEST["Payload"]),8,8));

// cpx is optionnal data
$cpx=hexTo32Integer(substr(htmlspecialchars($_REQUEST["Payload"]),16,8));

// alt is altitude in meters (from GPGGA sentence)
$array['alt']=0x1fff & $cpx >> 19;

// speed in kph (from GPRMC sentence)
$array['speed']=($cpx >> 12) & 0xff;
if ($array['speed'] > 102) $array['speed'] = ($array['speed'] - 94) * 16;
else if ($array['speed'] > 90) $array['speed'] = ($array['speed'] - 60) * 3;

// cap is course direction N,E,S,W from GPRMC sentence
$array['cap'] = ($cpx >> 10) & 3;

// bat is battery percent remainning voltage
$array['bat']=($cpx >> 3 ) & 0xff;

/* mod is message type (MSG_POSITION = 0-2, MSG_OPTION = 3, MSG_NO_MOTION = 4,
   MSG_NO_GPS = 5, MSG_MOTION_ALERT = 6, MSG_WEAK_BAT = 7) */
$array['mod']=$cpx & 7;

// Decompress altitude value. After 4096m, each values represent 16m. Maximum altitude
// measured during weather balloon journey on October 2016 has been 32544m.
if ($array['alt'] > 4096 && $array['mod'] < 3) $array['alt'] = ($array['alt']-3840)*16;

header('Content-type: application/json');
echo json_encode($array, JSON_PRETTY_PRINT);

```
