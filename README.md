# DJI Drone Hijacking PoC 

This is a repository for proof of concept of DJI Mini SE Hijacking explained in our paper:

[Behind The Wings: The Case of Reverse Engineering and Drone Hijacking in DJI Enhanced Wi-Fi Protocol](https://arxiv.org/abs/2309.05913)

## Getting Started

```
[ Linux PC ]      [ Router ]     [ DRONE ]
puppeteer.py <---> puppet.c <---> DJI Mini SE
```
- In `drone.py` change the `src_mac, dst_mac, bssid_mac` according to the victim controller and drone, also change the `key` to the correct key obtained. The `arp` and `beacon` is the raw unmodified UDP packet sent from remote to the drone collected from sniffing the communication.
- Compile `puppet.c` and run it at OpenWRT router that has Ath9K chip with 5Mhz monitor mode support.
- Run  `puppeteer.py`, make sure `c_host` variable is the address of the router.
- Hijack and control the drone with keyboard input, see `pupeteer.py`.
