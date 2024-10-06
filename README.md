[![Generic badge](https://img.shields.io/badge/Version-1.0.0-<COLOR>.svg)]()

This CBun is designed to facilitate the control and monitoring of grippers through digital I/Os. Originally developed by Kassow Robots, this CBun is primarily tailored for use with the GripKit-CR-Easy gripper from Weiss Robotics. It's important to note that the CBun is primarily intended for sample purposes.

# Usage

## Installation

Install **Weiss Robotics GRIPKIT** from the USB by the use of "+" button located within the Settings/CBuns.

<br/>

## Configuration

The GripKit CR Easy gripper can be connected using the M8 8-pole male connector on the ToolIO. Robot generation needs to be selected during activation to provide to correct pin mapping.

### Mapping

| Gripper  | Gen 1 | Gen 2  |       Description       |
| -------- | ----- | ------ | ----------------------- |
| **GND**  |  GND  |  GND   | ground                  |
| **VCC**  | TPSU2 | TDO06M | power (24V)             |
| **IN1**  | TDO04 | TDO07M | grip input (0/24V)      |
| **IN0**  | TDO03 | TDO08M | activate input (0/24V)  |
| **OUT1** | TAID4 | TDO04M | gripped output (0/24V)  |
| **OUT0** | TAID3 | TDO05M | no error output (0/24V) |

### Mounting

Mounting option allows to set the system toolload variable (LOAD1) with the mass and inertia of the gripper. Default (based on manufacturer specification, without fingers) or custom value can be used.

<br/>

## Published Methods 

```
grip(bool blocking, [optional] Load payload)
```
Move to the predefined grip (no part limit) position. Optionally wait until the motion is completed. 

Optionally set the system payload variable (LOAD2) if gripper detects a part (no_part_limit not reached), otherwise (no_part_limit reached or no payload specified) clear LOAD2. Load set in relation to the tool flange.

**blocking** block the method until the gripper is in state HOLDING or NO_PART or sooner if the call was interrupted by another call or IDLE_OR_ERROR state.

**payload** optional payload, the value is used to update the LOAD2 system variable if and when gripper detects a part.

<br/>

```
release(bool blocking)
```

Move to the predefined release position. Optionally wait until the motion is completed. Clear the system payload variable (LOAD2).

**blocking** block the method until the gripper is in state RELEASED or sooner if the call was interrupted by another call or IDLE_OR_ERROR state.

<br/>

## Published functions

```
Number isReleased()
```

Return 1 if gripper status is RELEASED, 0 otherwise.

<br/>

```
Number isHolding()
```

Return 1 if gripper status is HOLDING, 0 otherwise.

<br/>

```
Number isNoPart()
```

Return 1 if gripper status is NO_PART, 0 otherwise.

<br/>

```
Number isError()
```

Return 1 if gripper status is IDLE_OR_ERROR, 0 otherwise.

<br/>
<br/>

# Building

## Git Clone on Windows

When using `git clone` command on Windows, the git may convert Unix-style `\n` line breaks to Windows-style `\r\n` line breaks on checking files out. Since the CBun build container is Unix based, this automatic line break conversion may cause issues during the CBun development. Therefore make sure that this behaviour is disabled prior cloning this CBun example project by running the following command: 

`git config --global core.autocrlf false`

<br/>

## Build Environment

The CBun comes with the preconfigured CBun build environment which is based on the **Visual Studio Code Dev Containers** extension. With this you can open, debug and build the CBun project in Visual Studio Code on Windows, MacOS or Linux without the need for complicated build environment configuration. 

### Prerequisities

To get started, follow these steps:

1. Install and configure [Docker](https://www.docker.com/get-started) for your operating system, using one of the paths below.

   **Windows / macOS:**

   1. Install Docker [Desktop for Windows/Mac](https://www.docker.com/products/docker-desktop).

   2. If you are using Windows, enable WSL and make sure it is up to date by running the following Command Prompt: `wsl --update`

   **Linux:**

   1. Follow the [official install instructions for Docker CE/EE for your distribution](https://docs.docker.com/install/#supported-platforms).

   2. Add your user to the docker group by using a terminal to run: `sudo usermod -aG docker $USER`

   3. Sign out and back in again so your changes take effect.

2. Install [Visual Studio Code](https://code.visualstudio.com/) or [Visual Studio Code Insiders](https://code.visualstudio.com/insiders/).

3. Install the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers). 

### CBun Project Setup

To setup the CBun project in Visual Studio Code, follow these steps:

1. Open your CBun project folder in Visual Studio Code.

2. Click **Yes, I trust the authors**.

3. Click **Reopen in Container** (this may take few minutes).

5. Select the **GCC 8.4.0 x86_64-linux-gnu** as the active Kit.

### CBun Tasks

Finally you can run the following tasks from the Visual Studio Code:

* **CBun: Build** - Builds and assembles your CBun from sources into the `build/device_example.cbun` file.

* **CBun: Build and Install** - Builds and installs your CBun (for integration tests only)

<br/>

## CBun Assembly

The CBun installer (`weiss_gripkit.cbun`) is generated by the **CBun: Build** task in Visual Studio Code if the **Release** build variant is selected. Once the CBun is generated, it can be installed on the real robot from a USB stick or Google Drive.

Follow the steps bellow in order to generate and install the CBun:

1. Open the backend project in Visual Studio Code. 

2. Set **Release** build variant.

3. Run the **CBun: Build** task (**Terminal** -> **Run Task...** -> **CBun: Build**).

4. Copy the CBun installer (**PROJECT_FOLDER/build/device_example.cbun**) to a USB stick.

5. Install the CBun on a real robot from the USB stick.
