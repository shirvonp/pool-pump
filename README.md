# Introduction
This project aims to use accelerometer and gyro data from a pump mounted MPU6050 sensor to determine the operating conditions of a filter pump system.

# Equipment Used
*  Intex 1000GPH above ground pool pump/filter
*  Wemos D1 Mini
*  MPU6050 Triple Axis Accelerometer and Gyro Breakout
*  Micro USB power supply for Wemos D1 mini
*  SD Card Reader (Optional)
*  Raspberry Pi

# Mode of operation
The model is designed to predict four classes of pump operation:
* Normal operation with no skimmer connected
* Skimmer connected and operating correctly
* Skimmer flow blocked (restricted pump suction)
* Filter cartridge dirty (restricted pump discharge)

The system comprises of three separate python scripts:
## Datalogging
This script when run, sends a MQTT message to the Wemos D1 mini to initiate the sampling and sending of data from the MPU6050.
The absolute values of the accelerometer and gyro axes (accel x,y,z & gyro x,y,z) are sent via MQTT where it’s combined and saved as a csv file in the corresponding class folder (each csv file contains 4000 samples).

## Training
The training script reads the csv files saved in the four class folders and transforms them into a NumPy array of training data with window sizes of 6x100 (3 accel + 3 gyro readings  X 100 samples).
The data is scaled using MinMaxScaler and its values saved for use within the inference script.

Model - The model consists of four convolution layers followed by two dense layers with the output layer consisting of four neurons corresponding to the four classes.
After training the model is saved as an .h5 file for use by the inference script.

## Inference
The inference script sends a sample request MQTT message to the Wemos and waits for the sample data, this is then transformed using the same data processing pipeline used for training. Scaling is done by using the MinMaxScaler values (saved as pickle file) produced during training data processing ensuring the inference data is pre-processed exactly the same as the data the model was trained on.
The model is loaded from file and prediction is done on the sample, producing 39 predictions. The ‘mode’ of these predictions is calculated to identify the most common class prediction and this is used as the final output.
The predicted class is then published via MQTT to be read by any device that subscribes to the prediction topic (in my case this is OpenHab but can be a cloud based MQTT service or any other monitor that uses MQTT).
