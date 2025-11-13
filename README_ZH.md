#首先，安装Vmware Workstation

使用https://www.vmware.com/products/desktop-hypervisor/workstation-and-fusion 
创建账号并下载安装包

#其次，安装Ubuntu

使用https://releases.ubuntu.com/noble/ubuntu-24.04.3-desktop-amd64.iso
在Vmware中安装一个Ubuntu系统。

#启动虚拟化引擎

勾选启动虚拟化引擎

<img width="528" height="254" alt="屏幕截图 2025-11-14 000616" src="https://github.com/user-attachments/assets/133299df-8380-4147-8b9c-03cf739dc521" />

如果启动时显示不支持虚拟化

1.检测Windows是否启用虚拟化

<img width="715" height="370" alt="屏幕截图 2025-11-13 235717" src="https://github.com/user-attachments/assets/af62f6d8-02fa-4fa9-a0b1-be6113bc5969" />

2.关闭Windows内存完整性防护

<img width="905" height="590" alt="屏幕截图 2025-11-14 000033" src="https://github.com/user-attachments/assets/012046ac-d670-4cdb-83cb-90bf246e0453" />

若已经可以启动虚拟化引擎，则启动虚拟机

否则进一步检查系统功能

#安装docker

参照https://blog.csdn.net/u011278722/article/details/137673353

安装一个可用的docker

#拉取星绽docker映像

在Ubuntu中执行命令

docker pull asterinas/asterinas:0.16.1-20250922

#下载星绽源文件

在Ubuntu中执行命令

git clone https://github.com/asterinas/asterinas

注意：在Windows系统下拉取时内部文件会使用Windows风格的命令，无法在Linux中运行。一定要在Linux中执行拉取命令

#执行构建

在asterinas文件夹下运行命令

docker run -it --privileged --network=host   --device=/dev/kvm   -v $(pwd):/root/asterinas   asterinas/asterinas:0.16.1-20250922

来启动docker

在docker内部执行命令

make build

make run

#梯子

在Linux内进行任何下载操作时可能出现无法连接问题

建议使用Clash Verge Rev v2.4.3配合任意梯子api

Clash启动服务模式和Tun模式来将虚拟机内部的通信进行代理






