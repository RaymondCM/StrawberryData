# Strawberry Data

Data collection suite for strawberry data collection project. The standards document and example implementation can be found [here](Strawberry_Data_Collection.pdf).

## Controls 

| Key | Description |
| --- | ----------- |
| `save`, `s`, `Enter Key` | Writes all output to disk |
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
| `gui-enabled` | If true all connected camera streams are displayed on screen |
| `stabilise-exposure` | Throws away `stabilise-exposure-count` number of frames to stabilise the auto exposure |
| `stabilise-exposure-count` | Parameter used when `stabilise-exposure` is true | 
| `stream-colour` | Parent property controlling stream parameters for colour sensors (see `width`, `height` and `frame-rate`) |
| `stream-depth` | Parent property controlling stream parameters for depth sensors (see `width`, `height` and `frame-rate`) |
| `width` | Sensor resolution width |
| `height` | Sensor resolution height |
| `frame-rate` | Sensor resolution frame rate |
