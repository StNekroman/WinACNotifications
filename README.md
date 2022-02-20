# WinACNotifications
Small tool to execute custom code on laptop when it receives power supply or disconnects from it.    
How to run:

```shell
WinACNotifications.exe --nosleep --blocking --onstart online --online "app or script to run only power connected"
```

List of all possible options:  
`--online` cmd line to execute when AC brings online  
`--offline` cmd line to execute when AC becomes disconnected/offline  
`--offlinetimeout` optional timeout (in seconds) for running from battery, `--offline` action will be performed only after this timeout, if specified  
`--onlinetimeout` optional timeout (in seconds) for waiting on active AC, `--online` action will be performed only after this timeout, if specified  
`--runonce` will terminate WinACNotifications.exe when event hit, but raising required process  
`--blocking` if specified, then on online/offline changes previous task (with whole process tree) will be killed, rather then spawning new and new processes  
`--nosleep` prevents PC from hybernation/sleeping on start (on exit - removes nosleep block)  
`--logfile` path to log file to generate and append log lines there (about AC statuses changes and errors)  
`--onstart` initial action to perform, can be:  
1. `online` - do, what is declared in `--online`, if AC is online
2.  `offline` - do, what is declared in `--offline`, if AC is offline
3. or just do something else, like `--onstart "ping 127.0.0.1"`

## Download
Build yourself or use prebuild win x64 [WinACNotifications.exe](dist/WinACNotifications.exe "WinACNotifications.exe").

## Use cases
 Mining on laptop, of course!  
 When AC disconnects - it is right choise to stop mining from (not-extracted) battery.  
 Here is example usage:
```shell
WinACNotifications.exe --nosleep --blocking --onstart online --online "<path-to>\miner`.exe" --offlinetimeout 300
```
It does:
- starts
- prevents PC from sleep
- checks for AC, if online - starts miner
- on offline AC - waits for 5 minutes
- if AC is still offline - shuts down the miner
- when AC back online - starts miner immediately (becuase --onlinetimeout is not set) 

Or you can use in any other kind of automation.

## License
MIT