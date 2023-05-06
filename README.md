# ESP_IOT_Project
 
# Flow Charts:
This flow chart section contains main functions for easy understanding.
## Wifi Manager

```mermaid
flowchart TD
	ReadFiles --> Empty
    Empty -->|YES| AccesPoint
    Empty -->|NO| ConnectNetwork[Connect To Network]
    ConnectNetwork --> Success{Success ?}
    Success -->|YES| AccesWebPage[Accessing Web Page]
    Success -->|NO| AccesPoint
    AccesPoint --> WifiMan[WifiManager.html]
    WifiMan --> SaveFile[Saveing to SPIFFS]
    SaveFile --> Restart[Restart]
    Restart --> ReadFiles
	

ReadFiles[Reading Files]
Empty{Empty ?}
AccesPoint[Set Acces Point]
```
