<#
  This script installs and setup CouchDB.

  If the script is copied and run from a temporary folder (like when running via vagrant)
    the -originalBuildScriptPath parameter should be passed with the path to the original
    "provisioning" folder
#>
param ( # default to script path if no parameter is given
    [string]$originalBuildScriptPath = (Split-Path -parent $PSCommandPath)
)

Import-Module "$($originalBuildScriptPath)/Provisioning.psm1" -Force

$couchDBInstallerURL = "http://archive.apache.org/dist/couchdb/binary/win/2.3.0/couchdb-2.3.0.msi"
$couchDBInstaller = Join-Path $originalBuildScriptPath "couchdb-2.3.0.msi"

# Download msi file
try {
    Write-OutPut "Downloading CouchDB $couchDBInstallerURL"
    iwr $couchDBInstallerURL -OutFile $couchDBInstaller
} catch {
    Write-OutPut "ERROR: Couldn't download CouchDB installer from the specified location."
    exit 1
}

# Install couchdb
$msiexec = "msiexec.exe"
Invoke-Command $msiexec "/i $couchDBInstaller /passive"

# Set-up CouchDB to run as a single node server as described
# here: https://docs.couchdb.org/en/stable/setup/single-node.html
iwr -Method PUT -Uri http://127.0.0.1:5984/_users 
iwr -Method PUT -Uri http://127.0.0.1:5984/_replicator
iwr -Method PUT -Uri http://127.0.0.1:5984/_global_changes

# Replace the default listening port
# By default, CouchDB will be installed at C:\CouchDB and we need admin privileges.
#$couchDBConfigFile = Join-Path (Join-Path "C:\CouchDB" "etc") "default.ini"
#((Get-Content -path $couchDBConfigFile -Raw) -replace "5984","25984") | Set-Content -Path $couchDBConfigFile
# In addition to that, we must restart CouchDB in order for the changes to take effect.
#$net = "net.exe"
#Invoke-Command $net "stop 'Apache CouchDB'"
#Invoke-Command $net "start 'Apache CouchDB'"

exit 0
