#include <odroid_go.h>
#include <driver/adc.h>
#include "sensors/Wire.h"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define RESISTANCE_NUM 2
#define DEFAULT_VREF 1100
static esp_adc_cal_characteristics_t adc_chars;

#define TA_SHIFT 8 //Default shift for MLX90640 in open air
const byte MLX90640_address = 0x33;
static float mlx90640To[768];
paramsMLX90640 mlx90640;

BluetoothSerial serialBT;

bool dooverlay=true,saved=false,havesd=false;
int boxx=16,boxy=12,cmap=0,cr,cg,cb;
long gottime,firstsave=-1,battime;
float ambient,battery,fever=36,mn,mx;

void setup()
{
  Serial.begin(115200); // MUST BE BEFORE GO.BEGIN()!!!!!
  serialBT.begin("GO IR Camera");
  GO.begin();
  Wire.begin(15,4);
  Wire.setClock(400000);
  Serial.println("Starting...");
  Serial.println();
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
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
  GO.lcd.drawString("IR Camera v1.1",160,80);
  GO.lcd.setTextFont(2);
  GO.lcd.drawString("by Andrew Thomas",160,150);
  GO.lcd.drawString("2019-2020",160,170);
  GO.lcd.setTextFont(4);
  delay(1000);
  Serial.println("Getting initial IR frame...");
  getirframe();
  calcminmax();
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
      case '0': cmap=0; break;
      case '1': cmap=1; break;
      case '2': cmap=2; break;
      case 'a': sendtoserialtext();
                break;
      case 'b': sendtoserialbinary();
                break;
      case 'd': getirframe();
                calcminmax();
                break;
      case 'n': makebeep(2000,500);
                break;
      case 'o': dooverlay=true;
                break;
      case 's': savetosdcard();
                saved=true;
                break;
      case 'u': drawtodisplay(true);
                break;
      case 'v': serialBT.println("IR Camera v1.1");
                break;
      case 'z': dooverlay=false;
                break;
    }
  }
  GO.update();
  if(GO.BtnA.isPressed()==1)
  {
    getirframe();
    calcminmax();
    drawtodisplay(true);
    if(mx>=fever && cmap==2) makebeep(3000,1000);
  }
  if(GO.BtnMenu.isPressed()==1)
  {
    while(GO.BtnMenu.isPressed()==1){GO.update();delay(50);}
    if(dooverlay==true) dooverlay=false;
    else dooverlay=true;
    GO.lcd.clearDisplay();
    drawtodisplay(true);
  }
  if(GO.BtnVolume.isPressed()==1)
  {
    while(GO.BtnVolume.isPressed()==1){GO.update();delay(50);}
    cmap++;
    if(cmap>2) cmap=0;
    drawtodisplay(false);
  }
  if(GO.BtnSelect.isPressed()==1)
  {
    while(GO.BtnSelect.isPressed()==1){GO.update();delay(50);}
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
  if((millis()-battime)>30000) updatebattery();
}

void updatebattery()
{
  readbattery();
  drawbattery();
}

void calcminmax()
{
  int c;
  mn=99999,mx=-99999;
  for(c=0;c<768;c++)
  {
    if(mlx90640To[c]>mx) mx=mlx90640To[c];
    if(mlx90640To[c]<mn) mn=mlx90640To[c];
  }
  mn=int(mn);
  mx=int(mx+1);
  if(mn<-30) mn=-30;
  if(mx>300) mx=300;  
}

void drawtodisplay(bool cls)
{
  float mid,val;
  uint16_t x,y,ind,r,g,b,col;
  uint16_t xw=10,yw=10,xoff=0,yoff=0;
  if(dooverlay==true)
  {
    xw=7;
    yw=7;
    xoff=0;
    yoff=0;
  }
  mid=mlx90640To[((23-boxy)*32)+boxx];
  if(cls==true) GO.lcd.clearDisplay();
  for(y=0;y<24;y++)
  {
    for(x=0;x<32;x++)
    {
      ind=(y*32)+x;
      val=mlx90640To[ind];
      docolourmapping(val,mn,mx);
      col=GO.lcd.color565(cr,cg,cb);
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
    GO.lcd.drawNumber(int(mx),255,yoff+15);
    GO.lcd.setTextColor(CYAN,BLACK);
    GO.lcd.drawNumber(int(mn),255,yoff+(24*yw)-2);
    GO.lcd.setTextColor(WHITE,BLACK);
    GO.lcd.drawFloat(mid,1,255,yoff+((24*yw)/2)+yw);
    GO.lcd.drawRect(xoff+(boxx*xw),yoff+(boxy*yw),xw,yw,WHITE);
    // Draw button labels
    GO.lcd.setTextFont(2);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(BLACK,WHITE);
    GO.lcd.setTextDatum(ML_DATUM);
    GO.lcd.drawString(" ZOOM ",0,230);
    switch(cmap)
    {
      case 0: GO.lcd.drawString(" B R ",82,230);
      break;
      case 1: GO.lcd.drawString(" Y R ",82,230);
      break;
      case 2: GO.lcd.drawString(" FEV ",82,230);
      break;
    }
    if(saved==false && havesd==true)
    {
      GO.lcd.setTextDatum(MR_DATUM);
      GO.lcd.drawString(" SAVE  ",320,230);
    }
    GO.lcd.setTextFont(2);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(WHITE,BLACK);
    GO.lcd.drawString("Ambient: "+String(ambient,1)+" C",2,195);
  }
  drawbattery();
}

void drawbattery()
{
  int c,bat,nb,xoff=320-44,yoff=194;
  uint16_t col;
  if(dooverlay==false) yoff+=30;
  col=GO.lcd.color565(0,0,0);
  GO.lcd.fillRect(xoff-4,yoff-4,44,18,col);
  bat=int(map(constrain(battery,3.2,4),3.2,4,0,100));
  nb=bat/20;
  if(nb<=1)
  {
    nb=1;
    col=GO.lcd.color565(255,0,0);
  }
  else col=GO.lcd.color565(0,255,0);
  for(c=0;c<nb;c++)
    GO.lcd.fillRect(xoff+(c*7),yoff,5,10,col);
  col=GO.lcd.color565(255,255,255);
  GO.lcd.drawLine(xoff-3,yoff-3,xoff+36,yoff-3,col);
  GO.lcd.drawLine(xoff-3,yoff+12,xoff+36,yoff+12,col);
  GO.lcd.drawLine(xoff-3,yoff-3,xoff-3,yoff+12,col);
  GO.lcd.drawLine(xoff+36,yoff-3,xoff+36,yoff+2,col);
  GO.lcd.drawLine(xoff+36,yoff+7,xoff+36,yoff+12,col);
  GO.lcd.drawLine(xoff+39,yoff+2,xoff+39,yoff+7,col);
  GO.lcd.drawLine(xoff+36,yoff+2,xoff+39,yoff+2,col);
  GO.lcd.drawLine(xoff+36,yoff+7,xoff+39,yoff+7,col);
}

void docolourmapping(float t,float mn,float mx)
{
  switch(cmap)
  {
    case 0: // Red/blue mapping
            cr=int(map(t,int(mn),int(mx),0,255));
            cg=0;
            cb=int(map(t,int(mn),int(mx),255,0));
            break;
    case 1: // Yellow mapping
            cr=255;
            cg=int(map(t,int(mn),int(mx),255,0));
            cb=0;
            break;
    case 2: // Fever mapping
            if(t<=fever)
            {
              cr=0;
              cg=0;
              cb=int(map(t,int(mn),fever,255,50));
            }
            else
            {
              cr=int(map(t,fever,int(mx),50,255));
              cg=0;
              cb=0;
            }
            break;
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
    ambient=Ta;
  }
  gottime=millis();
  boxx=16,boxy=12;
  saved=false;
  readbattery();
}

void sendtoserialtext()
{
  int x,y,ind;
  float val;
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
      val=mlx90640To[ind];
      serialBT.print(val,2);
      serialBT.print(',');
    }
  }
  serialBT.print(ambient,2);
  serialBT.print(',');
  serialBT.println(battery,2);
}

void sendtoserialbinary()
{
  int x,y,ind,val;
  serialBT.write(254);
  serialBT.write(254);
  for(y=0;y<24;y++)
  {
    for(x=0;x<32;x++)
    {
      ind=(y*32)+x;
      val=int((mlx90640To[ind]+50)*100);
      serialBT.write(highByte(val));
      serialBT.write(lowByte(val));
    }
  }
  val=int((ambient+50)*100);
  serialBT.write(highByte(val));
  serialBT.write(lowByte(val));
  val=int(battery*100);
  serialBT.write(highByte(val));
  serialBT.write(lowByte(val));
  serialBT.write(255);
  serialBT.write(255);
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
  int x,y,ind;
  float val;
  File file=SD.open("/goircam.csv",FILE_APPEND);
  if(!file)
  {
    Serial.println("Failed to open file for appending");
    makebeep(500,500);
    return;
  }
  file.print(gottime,DEC);
  file.print(',');
  for(y=0;y<24;y++)
  {
    for(x=0;x<32;x++)
    {
      ind=(y*32)+x;
      val=mlx90640To[ind];
      file.print(val,2);
      file.print(',');
    }
  }
  file.println(ambient,DEC);
  file.close();
}

void makebeep(int pitch,int duration)
{
  GO.Speaker.setVolume(100);
  GO.Speaker.begin();
  GO.Speaker.tone(pitch,duration);
  delay(duration);
  GO.Speaker.end();
  delay(100);
}

void readbattery()
{
  int c;
  battery=0;
  for(c=0;c<10;c++)
    battery+=float(esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc_chars)) * RESISTANCE_NUM / 1000;
  battery/=10;
  battime=millis();
}
