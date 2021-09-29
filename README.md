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


# How to use

## Datalogging Hardware
* Connect the MPU6050 to the Wemos Di mini using the I2C connections.
![Pump](images/pump.jpg)
* Load "CSV_esp8266_MQTT_Data_G.ino" to the Wemos using the Arduino IDE.

## Data Collection
* Create a "MQTT Data" folder containg four folders named 1,2,3 & 4 to store files for each of these classes.
* Run the code in "MQTT Datalogger.ipynb", be sure to change the class number to corrispond to the class being recorded. Ecah time this is run a CSV file is generated with 4000 data points.

## Training
* Run the training file "Pump Vibration Training.ipynb" and monitor the results graph to ensure the model is not over fitting.
* The model will be saved at the end of training to the file "model_ex.h5".

## Inference 
* This can be saved as a .py file and run periodically on a respberry pi to provide results as often as required.



