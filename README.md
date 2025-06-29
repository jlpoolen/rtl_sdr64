# RTL-SDR 64-bit IQ Recorder

A long-duration IQ sample recorder for RTL-SDR dongles that supports files larger than 4 GiB by using 64-bit counters and file operations.  The current rtl_sdr only supports output of 4 GiB, so any attempt to record the IQ stream longer than 15 minutes will result in a truncated file.  This project will allow you to record a larger sample, e.g. 1 hour at 75 GiB.

## Disclosure
This code was generated by OpenAI's ChatGPT4.0 at my request under my supervision.  You are warned.

## Features

- Fully supports IQ recordings larger than 4 GiB.
- Timestamped and gain-tagged filenames for easy cataloging.
- Sidecar `.txt` file with metadata: start time, sample rate, gain, etc.
- All capture settings are command-line configurable.
- Clean C source and simple `Makefile`.

## Quick Start
### PreRequisite

Requires librtlsdr installed.
- Debian/Ubuntu: librtlsdr-dev - Software defined radio receiver for Realtek RTL2832U (development)
- Gentoo: ```net-wireless/rtl-sdr```

### Build

```bash
make
```


Example Run:
```./rtl_sdr64_friendly \
  --sample_rate 2400000 \
  --frequency 119100000 \
  --gain 402 \
  --device 0 \
  --time_seconds 3600 \
  --output_dir /tmp \
  --name myATC
```

Produces:

    /tmp/myATC_20250531_Sat_1430_2400000s_402g.iq

    /tmp/myATC_20250531_Sat_1430_2400000s_402g.iq.txt

Metafile Example:
```provenance: ./rtl_sdr64 built on May 31 2025
epoch_start: 1748730602
time_start_local: Sat May 31 14:30:02 2025
sample_rate: 2400000
frequency: 119100000
gain: 402
duration_sec: 3600
filename: /tmp/myATC_20250531_Sat_1430_2400000s_402g.iq
```
Permissions Tip

If you see a usb_claim_interface error -6, make sure no other process (e.g., openwebrx, rtl_tcp, etc.) is using the device. You may also need to create a udev rule to allow non-root access.

License: MIT
