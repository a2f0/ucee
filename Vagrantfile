Vagrant.configure("2") do |config|
    config.vbguest.auto_update = false
    config.vm.box = "archlinux/archlinux"
    config.vm.provider 'virtualbox' do |v|
      v.gui = false
      v.customize ["modifyvm", :id, "--graphicscontroller", "vmsvga"]
      v.customize ["modifyvm", :id, "--vram", "4"]
      v.customize ["modifyvm", :id, "--vrde", "off"]
      v.customize ['modifyvm', :id, "--clipboard", "bidirectional"]
      v.customize ['modifyvm', :id, "--audio", "none"]
      v.memory = 2048
      v.cpus = 2
    end
    config.vm.synced_folder ".", "/vagrant", type: "rsync"
    config.vm.provision "ansible" do |ansible|
        ansible.playbook = "ansible/playbook-arch-linux-vm.yaml"
      end
  end
  