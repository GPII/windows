# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.require_version ">= 1.8.0"

# By default this VM will use 2 processor cores and 2GB of RAM. The 'VM_CPUS' and
# "VM_RAM" environment variables can be used to change that behaviour.
cpus = ENV["VM_CPUS"] || 2
ram = ENV["VM_RAM"] || 2048

Vagrant.configure(2) do |config|

  config.vm.box = "inclusivedesign/windows10-eval"
  config.vm.guest = :windows

  config.vm.communicator = "winrm"
  config.winrm.username = "vagrant"
  config.winrm.password = "vagrant"
  config.vm.network :forwarded_port, guest: 3389, host: 3389, id: "rdp", auto_correct:true
  config.vm.network :forwarded_port, guest: 5985, host: 5985, id: "rdp", auto_correct:true

  config.vm.provider :virtualbox do |vm|
    vm.gui = true
    vm.customize ["modifyvm", :id, "--memory", ram]
    vm.customize ["modifyvm", :id, "--cpus", cpus]
    vm.customize ["modifyvm", :id, "--vram", "256"]
    vm.customize ["modifyvm", :id, "--accelerate3d", "off"]
    vm.customize ["modifyvm", :id, "--audio", "null", "--audiocontroller", "hda"]
    vm.customize ["modifyvm", :id, "--ioapic", "on"]
    vm.customize ["setextradata", "global", "GUI/SuppressMessages", "all"]
  end

  config.vm.provision "shell", inline: <<-SHELL
    choco upgrade firefox googlechrome -y
    # Copying custom Provisioning powershell module to vagrant tmp location.
    # I didn't find another way to load a custom Powershell module avoiding
    # the compactation of vagrant scripts in the tmp location.
    $moduleLocation = Join-Path $env:SystemDrive "vagrant/provisioning/Provisioning.psm1"
    $destinationDir = Join-Path $env:SystemDrive "tmp"
    cp $moduleLocation $destinationDir
  SHELL
  
  config.vm.provision "shell", path: "provisioning/Chocolatey.ps1"
  config.vm.provision "shell", path: "provisioning/Npm.ps1"
  config.vm.provision "shell", path: "provisioning/Build.ps1"

end
