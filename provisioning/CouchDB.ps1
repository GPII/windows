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

#Write-OutPut "Adding CouchDB to the system"
$couchDBInstallerURL = "http://archive.apache.org/dist/couchdb/binary/win/2.3.0/couchdb-2.3.0.msi"
$couchDBInstaller = Join-Path $originalBuildScriptPath "couchdb-2.3.0.msi"

# Download msi file
Write-OutPut "Downloading CouchDB from $couchDBInstallerURL"
try {
    $r1 = iwr $couchDBInstallerURL -OutFile $couchDBInstaller
} catch {
    Write-OutPut "ERROR: Couldn't download CouchDB installer from the specified location. Error was $_"
    exit 1
}

# Install couchdb
$msiexec = "msiexec.exe"
Write-OutPut "Installing CouchDB ..."
try {
    Invoke-Command $msiexec "/i $couchDBInstaller /passive"
} catch {
    Write-OutPut "ERROR: CouchDB couldn't be installed"
    exit 1
}

# Set-up CouchDB to run as a single node server as described
# here: https://docs.couchdb.org/en/stable/setup/single-node.html
Write-OutPut "Configuring CouchDB ..."
try {
    $r1 = iwr -Method PUT -Uri http://127.0.0.1:5984/_users
    $r2 = iwr -Method PUT -Uri http://127.0.0.1:5984/_replicator
    $r3 = iwr -Method PUT -Uri http://127.0.0.1:5984/_global_changes
} catch {
    Write-OutPut "ERROR: CouchDB couldn't be installed. Error was $_"
    exit 1
}

# Replace the default listening port
# By default, CouchDB will be installed at C:\CouchDB.
Write-OutPut "Changing default listening port to 25984 ..."
$couchDBConfigFile = Join-Path (Join-Path "C:\CouchDB" "etc") "default.ini"
((Get-Content -path $couchDBConfigFile -Raw) -replace "5984","25984") | Set-Content -Path $couchDBConfigFile

# In addition to that, we must restart CouchDB in order for the changes to take effect
Write-OutPut "Restarting CouchDB ..."
Restart-Service -Name "Apache CouchDB"

Write-OutPut "CouchDB is now installed and configured"
exit 0
