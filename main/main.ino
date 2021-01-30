#include <ArduinoModbus.h>
#include <SPI.h>
#include <SD.h>

ModbusRTUClientClass mod;
int pmCount = 10;
int regsCount = 11;
Sd2Card card;
SdVolume volume;
SdFile root;
// change this to match your SD shield or module;
// Default SPI on Uno and Nano: pin 10
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 10;


class PowerMeter {
    int id;
  public:
    PowerMeter(int id) {
      this->id = id;
    }

    int GetID() {
      return this->id;
    }

    void ReadRegisters(long values[]) {
      int addresses [] = {3925, 3939, 3953, 3929, 3943, 3957, 3913, 3923, 3937, 3951, 3907};
      for (int i = 0; i < regsCount; i++) {
        values[i] = mod.holdingRegisterRead(this->id, addresses[i]);
        delay(30);
      }
    }

    void WriteDataToFile() {
      String filePath = "/pm" + String(this->id) + ".csv";

      if (!SD.exists(filePath)) {
        this->CreateFile(filePath);
      }

      File f = SD.open(filePath, FILE_WRITE);
      String strToWrite = "";
      long regs[regsCount];
      this->ReadAll(regs);
      for (int i = 0; i < regsCount; i++) {
        strToWrite += regs[i];
        if (i < regsCount - 1)
          strToWrite += ",";
      }

      f.println(strToWrite);
      f.close();
    }

  private:
    void CreateFile(String filePath) {
      File f = SD.open(filePath, FILE_WRITE);
      f.println("time,v1,v2,v3,a1,a2,a3,a_avg,pf1,pf2,pf3,pf_avg");
      f.close();
      delay(50);
    }
};

void setup() {
  Serial.begin(9600);

  if (!card.init(SPI_HALF_SPEED, chipSelect)) {

    Serial.println("initialization failed. Things to check:");

    Serial.println("* is a card inserted?");

    Serial.println("* is your wiring correct?");

    Serial.println("* did you change the chipSelect pin to match your shield or module?");

    while (1);

  } else {

    Serial.println("Wiring is correct and a card is present.");

  }
  Serial.println();

  Serial.print("Card type:         ");

  switch (card.type()) {

    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;

    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;

    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;

    default:
      Serial.println("Unknown");
  }

  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");

  Serial.println(volume.clusterCount());

  Serial.print("Blocks x Cluster:  ");

  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");

  Serial.println(volume.blocksPerCluster() * volume.clusterCount());

  Serial.println();

  // print the type and size of the first FAT-type volume

  uint32_t volumesize;

  Serial.print("Volume type is:    FAT");

  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks

  volumesize *= volume.clusterCount();       // we'll have a lot of clusters

  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)

  Serial.print("Volume size (Kb):  ");

  Serial.println(volumesize);

  Serial.print("Volume size (Mb):  ");

  volumesize /= 1024;

  Serial.println(volumesize);

  Serial.print("Volume size (Gb):  ");

  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");

  root.openRoot(volume);

  // list all files in the card with date and size

  root.ls(LS_R | LS_DATE | LS_SIZE);

  root.close();
}

void loop() {
  for (int i = 1; i < pmCount + 1; i++) {
    PowerMeter pm(i);
    long regs[regsCount];
    Serial.print("PM #");
    Serial.print(pm.GetID());
    Serial.print(" -> v1:");
    pm.ReadAll(regs);
    Serial.println(regs[0]);

    pm.WriteCurrentToFile();
  }

  delay(1000);
}
