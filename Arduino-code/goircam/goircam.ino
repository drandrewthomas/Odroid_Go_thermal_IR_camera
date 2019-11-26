#include <odroid_go.h>
#include "sensors/Wire.h"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define TA_SHIFT 8 //Default shift for MLX90640 in open air
const byte MLX90640_address = 0x33;
static float mlx90640To[768];
paramsMLX90640 mlx90640;

BluetoothSerial serialBT;

bool dooverlay=true,saved=false,havesd=false;
int boxx=16,boxy=12;
long gottime,firstsave=-1;

void setup()
{
  Serial.begin(115200); // MUST BE BEFORE GO.BEGIN()!!!!!
  GO.begin();
  delay(500);
  serialBT.begin("GO IR Camera");

  // turn speaker off
  GO.Speaker.setVolume(0);
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  pinMode(26, OUTPUT);
  digitalWrite(26, LOW);

  Wire.begin(15,4);
  Wire.setClock(400000);
  Serial.println("Starting...");
  Serial.println();
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");
  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");
  MLX90640_SetRefreshRate(MLX90640_address,0x03);
  if(SD.begin()) havesd=true;
  else Serial.println("Card Mount Failed");
  GO.lcd.setTextFont(4);
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextDatum(MC_DATUM);
//  GO.lcd.setCursor(160,120);
  GO.lcd.drawString("IR Camera v1.0",160,120);
  delay(1000);
  getirframe();
  drawtodisplay(true);
  Serial.println("Setup done");
}

void loop()
{
  char inbyte;
  if(serialBT.available()>0)
  {
    inbyte=serialBT.read();
    switch(inbyte)
    {
      case 'd': getirframe();
                sendtoserialtext();
                break;
      case 'i': getirframe();
                drawtodisplay(true);
                break;
      case 't': sendtoserialtext();
                break;
    }
  }
  GO.update();
  if(GO.BtnA.isPressed()==1)
  {
    getirframe();
    drawtodisplay(true);
  }
  if(GO.BtnMenu.isPressed()==1)
  {
    while(GO.BtnMenu.isPressed()==1){GO.update();delay(50);}
    if(dooverlay==true) dooverlay=false;
    else dooverlay=true;
    GO.lcd.clearDisplay();
    drawtodisplay(true);
  }
  if(GO.BtnStart.isPressed()==1)
  {
    if(saved==false)
    {
      savetosdcard();
      saved=true;
      drawtodisplay(true);
    }
  }
  if(GO.JOY_Y.isAxisPressed()==1)
  {
    if(boxy<23 && dooverlay==true)
    {
      boxy++;
      drawtodisplay(false);
      delay(50);
    }
  }
  if(GO.JOY_Y.isAxisPressed()==2)
  {
    if(boxy>0 && dooverlay==true)
    {
      while(GO.JOY_Y.isAxisPressed()!=0){GO.update();delay(50);}
      boxy--;
      drawtodisplay(false);
      delay(50);
    }
  }
  if(GO.JOY_X.isAxisPressed()==1)
  {
    if(boxx<31 && dooverlay==true)
    {
      while(GO.JOY_X.isAxisPressed()!=0){GO.update();delay(50);}
      boxx++;
      drawtodisplay(false);
      delay(50);
    }
  }
  if(GO.JOY_X.isAxisPressed()==2)
  {
    if(boxx>0 && dooverlay==true)
    {
      while(GO.JOY_X.isAxisPressed()!=0){GO.update();delay(50);}
      boxx--;
      drawtodisplay(false);
      delay(50);
    }
  }
}

/* map an intensity from 0 to 255 to an RGB value using the NASA/IPAC colors using 6 equal steps */
uint16_t intensity_to_rgb(uint16_t col)
{
  uint16_t r,g,b;
  switch(col)
  {
    case   0 ...  41: r=0; g=0; b=map(col,0,41,20,140); break;
    case  42 ... 127: r=map(col,41,128,0,255); g=0; b=map(col,41,128,140,10); break;
    case 128 ... 169: r=255; g=map(col,128,170,0,60); b=map(col,128,170,10,0); break;
    case 170 ... 212: r=255; g=map(col,170,212,60,235); b=0; break;
    case 213 ... 255: r=255; g=map(col,212,255,235,255); b=map(col,212,255,0,255); break;
    default: r=g=b=0; // never happens
  }
  return GO.lcd.color565(r,g,b);
}

void drawtodisplay(bool cls)
{
  uint16_t c,x,y,ind,col;
  uint16_t xw=10,yw=10,xoff=0,yoff=0;
  float mn=99999,mx=-99999,mid,val;
  if(dooverlay==true)
  {
    xw=7;
    yw=7;
    xoff=0;
    yoff=0;
  }
  for(c=0;c<768;c++)
  {
    if(mlx90640To[c]>mx) mx=mlx90640To[c];
    if(mlx90640To[c]<mn) mn=mlx90640To[c];
  }
  mid=mlx90640To[((23-boxy)*32)+boxx];
  if (mid<-40) mid=-40;
  if (mid>300) mid=300;
  if(mn<-40) mn=-40;
  if(mx>300) mx=300;
  if(cls==true) GO.lcd.clearDisplay();
  for(y=0;y<24;y++)
  {
    for(x=0;x<32;x++)
    {
      ind=(y*32)+x;
      val=mlx90640To[ind];
      if (val<-40) val=-40;
      if (val>300) val=300;
      // map temp to 0..255
      col=int(map(int(val*1000),int(mn*1000),int(mx*1000),0,255));
      col=intensity_to_rgb(col);
      GO.lcd.fillRect(xoff+(x*xw),yoff+((24*yw)-(y*yw)),xw,yw,col);
    }
  }
  if(dooverlay==true)
  {
    if(cls==false) GO.lcd.fillRect(xoff+(32*xw)+5,0,320-(xoff+(32*xw)),210,BLACK);
    GO.lcd.setTextFont(2);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextDatum(ML_DATUM);
    GO.lcd.setTextColor(MAGENTA,BLACK);
    GO.lcd.drawFloat(mx,3,255,yoff+15);
    GO.lcd.setTextColor(CYAN,BLACK);
    GO.lcd.drawFloat(mn,3,255,yoff+(24*yw)-2);
    GO.lcd.setTextColor(WHITE,BLACK);
    GO.lcd.drawFloat(mid,3,255,yoff+((24*yw)/2)+yw);
    GO.lcd.drawRect(xoff+(boxx*xw),yoff+(boxy*yw),xw,yw,WHITE);
    // Draw button labels
    GO.lcd.setTextFont(2);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(BLACK,WHITE);
    GO.lcd.setTextDatum(ML_DATUM);
    GO.lcd.drawString(" ZOOM ",0,230);
    if(saved==false && havesd==true)
    {
      GO.lcd.setTextDatum(MR_DATUM);
      GO.lcd.drawString(" SAVE  ",320,230);
    }
  }
}

void getirframe()
{
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }
    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }
  gottime=millis();
  boxx=16,boxy=12;
  saved=false;
}

void sendtoserialtext()
{
  int x,y,ind,val;
  if(firstsave==-1)
  {
    firstsave=millis();
    serialBT.print("0");
  }
  else serialBT.print(gottime-firstsave,DEC);
  serialBT.print(',');
  for(y=0;y<24;y++)
  {
    for(x=0;x<32;x++)
    {
      ind=(y*32)+x;
      val=int((mlx90640To[ind]+50)*100);
      serialBT.print(val,DEC);
      if(x==31 && y==23) serialBT.println("");
      else serialBT.print(',');
    }
  }
}

boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

void savetosdcard()
{
  int x,y,ind,val;
  File file=SD.open("/goircam.csv",FILE_APPEND);
  if(!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  file.print(gottime,DEC);
  file.print(',');
  for(y=0;y<24;y++)
  {
    for(x=0;x<32;x++)
    {
      ind=(y*32)+x;
      val=int((mlx90640To[ind]+50)*100);
      file.print(val,DEC);
      if(!(x==31 && y==23)) file.print(',');
    }
  }
  file.println();
  file.close();
}

