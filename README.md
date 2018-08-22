# Strawberry Data

Data collection suite for strawberry data collection project. The standards document and example implementation can be found [here](Strawberry_Data_Collection.pdf).

## Controls 

| Key | Description |
| --- | ----------- |
| `save`, `s`, `Enter Key` | Writes all output to disk |
| `new`, `n` | Creates new dataset folder and asks for meta data input |
| `laser0`, `l0`  | Turns laser off |
| `laser1 <param>`, `l1 <param>`  | Turns laser on, \<param\> can be min(-3), mid(-2), max(-1) or any float value |
| `stab`, `st`  | Throws away frames for correcting exposure |
| `help`, `h`  | Displays help |
| `quit`, `q`  | Quits |

    
## Config 

Parameters are stored in the [`config.json`](config.json) file.

| Key | Description |
| --- | ----------- |
| `save-path-prefix` | Controls which folder the data structure is save in. Final path = `save-path-prefix` + data structure path |
| `project-name` | Top level filter folder for organising different data collection sessions. Final path = `save-path-prefix` + `project-name` + "/" |
| `gui-enabled` | If true all connected camera streams are displayed on screen, if true stabilise exposure can be false. |
| `stabilise-exposure` | Throws away `stabilise-exposure-count` number of frames to stabilise the auto exposure |
| `stabilise-exposure-count` | Parameter used when `stabilise-exposure` is true | 
| `stream-colour` | Parent property controlling stream parameters for colour sensors (see `width`, `height` and `frame-rate`) |
| `stream-depth` | Parent property controlling stream parameters for depth sensors (see `width`, `height` and `frame-rate`) |
| `width` | Sensor resolution width |
| `height` | Sensor resolution height |
| `frame-rate` | Sensor resolution frame rate |
| `options` | Parent property that houses global sensor parameters (see `auto-exposure`, `back-light-compensation` and `auto-white-balance`) |
| `auto-exposure` | Determines weather the sensor will determine exposure parameters using an internal algorithm |
| `back-light-compensation` | This setting when on will compensate for very bright backgrounds to ensure more uniform lighting |
| `auto-white-balance` | Determines weather the sensor can dynamically  calculate the white balance parameters |
| `file-names` | Contains all of the file names and extensions for multiple types (See for reference) |


## Example Usage

```bash
git clone https://github.com/RaymondKirk/StrawberryData
mkdir -p StrawberryData/build
cd StrawberryData/build
cmake ..
make all -j4
./grabber

# Application starts
# ...
# Controls:
# 	-save, s (Writes all output to disk)
# 	-laser0, l0 (Turns laser off)
# 	-laser1 <param>, l1 <param> (Turns laser on)
# 		-<param> can be min(-3), mid(-2), max(-1) or any float value
# 	-stab, st (Throws away frames for correcting exposure)
# 	-new, n (Creates new dataset)
# 	-help, h (Displays help)
# 	-quit, q (Quits)
# Enter Control:

# Enter the new command and start data collection
# When finished press q to quit and repeat from #Application Starts
```
