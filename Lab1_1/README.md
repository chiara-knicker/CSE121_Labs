# Setup Wifi
sudo vim /etc/netplan/50-cloud-init.yaml

edit file to look like this:
```
network:
    version: 2
    ethernets:
        eth0:
            dhcp4: true
            optional: true
    wifis:
        renderer: networkd
        wlan0:
            access-points:
                your_wifi_name:
                    password: your_password
            dhcp4: true
            optional: true
        wlan0:
            access-points:
                your_second_wifi_name:
                    password: your_password
            dhcp4: true
            optional: true
        wlan0:
            access-points:
                "eduroam":
                    auth:
                        key-management: eap
                        password: "your_password"
                        method: ttls
                        identity: "your_eduroam_email"
                        phase2-auth: "MSCHAPV2"
```

netplan generate  
netplan apply

check that the wifi networks show up:  
sudo wpa_cli -i wlan0 list_networks

Output should look like this:  
network id / ssid / bssid / flags  
0       your_wifi_name any  
1       your_second_wifi_name   any  
2       eduroam any     [CURRENT]  

[CURRENT] tells you which wifi you are connected to

You can also check which wifi you're connected to like this:  
iwgetid

Output should look like this:  
wlan0     ESSID:"wifi_name"


