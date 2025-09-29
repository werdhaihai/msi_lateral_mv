# msi_lateral_mv

This is a BOF that uses the MSI Server over DCOM to install and configure an ODBC driver using the Custom Action Server
Read more on the blogpost


```
Usage:
  msi_lateral_mv local <drivername> <dllpath>                                : Local, current user
  msi_lateral_mv local <user> <pass> <drivername> <dllpath>                  : Local, alternate user
  msi_lateral_mv local <domain> <user> <pass> <drivername> <dllpath>         : Local, domain user
  msi_lateral_mv remote <host> <drivername> <dllpath>                        : Remote, current user
  msi_lateral_mv remote <user> <pass> <host> <drivername> <dllpath>          : Remote, alternate user
  msi_lateral_mv remote <domain> <user> <pass> <host> <drivername> <dllpath> : Remote, domain user
```

The proof of concept relies on placing the DLL on the remote system. 
The code for the DLL can be found in https://github.com/werdhaihai/msi_lateral_mv/tree/main/sqldriverdll
