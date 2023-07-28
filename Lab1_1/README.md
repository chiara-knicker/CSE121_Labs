# Lab 1.1
## Setup Wifi
```
sudo vim /etc/netplan/50-cloud-init.yaml
```

Edit file to look like this:
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
Make sure to replace the wifi names, passwords and eduroam credentials with your own.

Apply changes:
```
netplan generate
```
```
netplan apply
```

## Check Connection
Check that the wifi networks show up
```
sudo wpa_cli -i wlan0 list_networks
```

Output should look like this:
```
network id / ssid / bssid / flags
0       your_wifi_name          any  
1       your_second_wifi_name   any  
2       eduroam any             [CURRENT]
```

[CURRENT] tells you which wifi you are connected to

You can also check which wifi you're connected to like this:  
```
iwgetid
```

Output should look like this:
```
wlan0     ESSID:"wifi_name"
```

You might have to install iwgetid first.

## Issues encountered
After editing the file, listing the networks returned this:
```
network id / ssid / bssid / flags
0       your_wifi_name          any  
1       your_second_wifi_name   any  
2       eduroam any             [TEMP-DISABLED]
```
It turned out that I used the wrong password. Changing the file, and running netplan generate and netplan apply again fixed it.
