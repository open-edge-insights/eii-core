# `OpenVINO FPGA toolkit installation and configuration setup guide`

**Pre-requisites**:
   * Running HDDL-F(FPGA) devices on Ubuntu 18.04 with Kernel above 5.3 is not supported by OpenVINO 2020.3 so please downgrade the kernel version
   * Please download and install OpenVINO FPGA toolkit version 2020.3(`OPENVINO_IMAGE_VERSION` used in [build/.env](build/.env)) using below links.
   * OpenVINO FPGA toolkit installation guide: https://docs.openvinotoolkit.org/2020.3/_docs_install_guides_installing_openvino_linux_fpga.html
   * FPGA configuration setup guide: https://docs.openvinotoolkit.org/2020.3/_docs_install_guides_VisionAcceleratorFPGA_Configure.html

* Download and install Intel Quartus Software lite version.
   * URL: [Click here to download the Quartus Software](https://fpgasoftware.intel.com/18.1/?edition=lite&platform=linux)
   * Scroll down and select Additional Software tab then click on Stand-Alone Software download icon.
   * Change the permission of the package to be executable
     ```sh
     $ chmod +x QuartusProgrammerSetup-18.1.0.625-linux.run
     ```
   * Execute the following command to install Quartus:
     ```sh
     $ sudo ./QuartusProgrammerSetup-18.1.0.625-linux.run
     ```
    **Note**: While Installing Quartus give the installation directory as /opt/intel/intelFPGA/18.1.

* The FPGA plugin works in heterogenous mode, so the device type in UDFs to be used would be `"HETERO:FPGA,CPU"` or `"HETERO:FPGA,GPU"`. For more details, refer: https://docs.openvinotoolkit.org/2020.3/_docs_IE_DG_supported_plugins_FPGA.html

## Connection and Installing FPGA drivers

1. Connect the FPGA card onto the target machine in PCI-E slot.
   * Check if the host system recognizes the Intel® Vision Accelerator Design with Intel® Arria® 10 FPGA board. Confirm you can detect the PCIe card.

      ```
      $ lspci | grep -i Altera
      ```
    Your output is similar to "01:00.0 Processing accelerators: Altera Corporation Device 2494 (rev 01)"

2. To setup FPGA drivers on host machine please refer the guide: [Click here for FPGA driver setup guide](https://docs.openvinotoolkit.org/2020.3/_docs_install_guides_VisionAcceleratorFPGA_Configure.html)

Note: Since Intel® Arria® 10 FPGA (Mustang-F100-A10) Speed Grade 1 is not available in the OpenVINO 2020.3 package(as mentioned in the above guide) we will not be able to support this as EIS uses OpenVINO version 2020.3 which cannot be downgraded to 2020.1(as a workaround mentioned in the guide).

