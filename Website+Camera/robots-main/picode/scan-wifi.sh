#!/usr/bin/env bash
echo 'Scanning for wifi connections'
NETWORK=$(iwlist wlan0 scan)

if [[ $NETWORK == *'Soele_5G'* ]]; then
   echo 'Found Soele_5G WiFi'
   WPA='ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=DK

network={
   ssid="Soele_5G"
   psk="REDACTED"
}
'
   echo "$WPA" > /etc/wpa_supplicant/wpa_supplicant.conf
elif [[ $NETWORK == *'Benjamin TP_Link'* ]]; then
   echo 'Found Benjamin TP_Link WiFi'
   WPA='ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=DK

network={
   ssid="Benjamin TP_Link"
   psk="REDACTED"
}
'
   echo "$WPA" > /etc/wpa_supplicant/wpa_supplicant.conf
else 
   echo 'Did not find home WiFi... Trying eduroam WiFi'
   WPA='ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=DK

network={
   ssid="eduroam"
   prioriy=1
   proto=RSN
   key_mgmt=WPA-EAP
   pairwise=CCMP
   auth_alg=OPEN
   eap=PEAP
   identity=REDACTED
   password=REDACTED
   phase1="peaplabel=0"
   phase2="auth=MSCHAPV2"
}
'
   echo "$WPA" > /etc/wpa_supplicant/wpa_supplicant.conf
fi

