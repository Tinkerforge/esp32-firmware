---
name: Bug report
about: Something is not working as it should
title: ''
labels: ''
assignees: ''
---
<!-- please alter/replace the text below to make your point clear -->

## Describe the bug

A clear and concise description of what the bug is.

## To Reproduce

Steps to reproduce the behavior:
1. The LED stripe on the charger where blinking green.
2. I plugged the charge cable into the car.
3. The car started to charge and
4. the LED stipe on the charger where pulsating green, but
4. the charging stopped after a few seconds.
5. The LEDs on the Charger where blinking yellow.

or:

1. Go to '...'
2. Click on '...'
3. Scroll down to '....'
4. See error

## Expected behavior

A clear and concise description of what you expected to happen.

## Screenshots & Logfiles

Please **always** add the logfile to help explain your problem.
You can get a `Debug report + event log` via the web UI on the `System` / `Event log` page.
Please download one from your box right after the problem happened and append it to this bug report.

*You can just drag and drop your `debug-report-AC011K-12345678901234-2023-01-28T13-55-19-290.txt` file into the bug report window.*

or you can copy paste your log in below (between the two lines starting with \`\`\`):

<details>
  <summary> see the debug report + event log </summary>

``` JSON5

2023-01-28T12-00-45-764Z
Scroll down for event log!

{"uptime": 3858171703,
 "free_heap_bytes":95044
}

2023-01-24 14:09:29,839  MQTT: Connected to broker.
2023-01-24 14:09:30,574  MQTT: Disconnected from broker.
2023-01-25 16:50:22,646  Charger state changed from 0 to 1
2023-01-25 16:50:31,646  Charger state changed from 1 to 2
2023-01-25 16:50:31,718  Tracked start of charge.
2023-01-25 16:50:33,723  Charger state changed from 2 to 3
2023-01-25 16:57:04,928  Charger state changed from 3 to 2
2023-01-25 16:57:17,965  Charger state changed from 2 to 3

```
</details>


If applicable (if it's a UI problem), add screenshots too.

*You can just drag and drop your image file into the bug report window.*


## Versions:

- Firmware version: <firmware-version> <!-- it is shown on the `System` / `firmware update` page -->
- Make and model of the car do you want to charge:

## Additional context

Add any other context about the problem here.
