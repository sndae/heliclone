using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace HelicloneMan
{
    class Eeprom
    {
        //
        // RADIO CONFIG
        //
        public Byte RC_Contrast
        {
            get
            {
                return eepromData[4];
            }
            set
            {
                eepromData[4] = value;
            }
        }

        public Byte RC_Backlight
        {
            get
            {
                return eepromData[4+1];
            }
            set
            {
                eepromData[4+1] = value;
            }
        }

        public Byte RC_KeyBeep
        {
            get
            {
                return eepromData[4 + 2];
            }
            set
            {
                eepromData[4 + 2] = value;
            }
        }

        public Byte RC_AlarmBeep
        {
            get
            {
                return eepromData[4 + 3];
            }
            set
            {
                eepromData[4 + 3] = value;
            }
        }

        public Byte RC_VolumeBeep
        {
            get
            {
                return eepromData[4 + 4];
            }
            set
            {
                eepromData[4 + 4] = value;
            }
        }

        public UInt16[][] RC_adc_c
        {
            get
            {
                UInt16[][] adc_c = new UInt16[8][];
                for (int i = 0; i < 8; i++)
                {
                    adc_c[i] = new UInt16[3];
                    adc_c[i][0] = 0;
                    adc_c[i][1] = 0;
                    adc_c[i][2] = 0;
                }

                for (int c = 0; c < 8; c++)
                {
                    for (int v = 0; v < 3; v++)
                    {
                        UInt16 b1 = (UInt16)eepromData[4 + 5 + (c * 6 + v*2)];
                        UInt16 b2 = (UInt16)eepromData[4 + 5 + (c * 6 + v*2 + 1)];
                        adc_c[c][v] = (UInt16)((b2 << 8) + b1);
                    }
                }

                return adc_c;
            }
            set
            {
                for (int c = 0; c < 8; c++)
                {
                    for (int v = 0; v < 3; v++)
                    {
                        eepromData[4 + 5 + (c * 6 + v * 2)] = (Byte)value[c][v];
                        eepromData[4 + 5 + (c * 6 + v * 2 + 1)] = (Byte)(value[c][v] >> 8);
                    }
                }
            }
        }

        public Byte RC_Message_box_timeout
        {
            get
            {
                return eepromData[4 + 4 + 24 * 2 + 1];
            }
            set
            {
                eepromData[4 + 4 + 24 * 2 + 1] = value;
            }
        }

        public Byte RC_VoltageWarning
        {
            get
            {
                return eepromData[4 + 4 + 24 * 2 + 2];
            }
            set
            {
                eepromData[4 + 4 + 24 * 2 + 2] = value;
            }
        }

        public Byte RC_SelectedModel
        {
            get
            {
                return eepromData[4 + 4 + 24 * 2 + 3];
            }
            set
            {
                eepromData[4 + 4 + 24 * 2 + 3] = value;
            }
        }

        //
        // MODEL CONFIG
        //

        private int Eeprom_Model_Offset()
        {
            return (int)(RC_SelectedModel) * 200 + 100;
        }

        public string MC_Name
        {
            get
            {
                string name = "";

                for (int i = 0; i < 10; i++)
                {
                    if (eepromData[Eeprom_Model_Offset() + i] != 0)
                    {
                        name = name + (char)eepromData[Eeprom_Model_Offset() + i];
                    }
                }

                return name;
            }
            set
            {
                for (int i = 0; i < 10; i++)
                {
                    try
                    {
                        eepromData[Eeprom_Model_Offset() + i] = (Byte)value[i];
                    }
                    catch (Exception)
                    {
                        eepromData[Eeprom_Model_Offset() + i] = (Byte)0;
                    }
                }
                eepromData[Eeprom_Model_Offset() + 9] = (Byte)0;
            }
        }

        public SByte[] MC_trim
        {
            get
            {
                SByte[] trim = new SByte[4];
                for (int i = 0; i < 4; i++)
                {
                    trim[i] = (SByte)eepromData[Eeprom_Model_Offset() + 11 + i];
                }

                return trim;
            }
            set
            {
                for (int i = 0; i < 4; i++)
                {
                    eepromData[Eeprom_Model_Offset() + 11 + i] = (Byte)value[i];
                }
            }
        }

        public SByte[] MC_subTrim
        {
            get
            {
                SByte[] trim = new SByte[8];
                for (int i = 0; i < 8; i++)
                {
                    trim[i] = (SByte)eepromData[Eeprom_Model_Offset() + 15 + i];
                }

                return trim;
            }
            set
            {
                for (int i = 0; i < 8; i++)
                {
                    eepromData[Eeprom_Model_Offset() + 15 + i] = (Byte)value[i];
                }
            }
        }

        public Byte MC_ModelType
        {
            get
            {
                return eepromData[Eeprom_Model_Offset() + 10];
            }
            set
            {
                eepromData[Eeprom_Model_Offset() + 10] = value;
            }
        }

        public Byte[] MC_servoDirection
        {
            get
            {
                Byte eeServoDir = eepromData[Eeprom_Model_Offset() + 103];
                Byte[] servoDir = new Byte[8];
                for (int i = 0; i < 8; i++)
                {
                    if ((eeServoDir & (1 << i)) == (1 << i))
                    {
                        servoDir[i] = 1;
                    }
                    else
                    {
                        servoDir[i] = 0;
                    }
                }

                return servoDir;
            }
            set
            {
                Byte eeServoDir = 0;
                Byte[] servoDir = value;

                for (int i = 0; i < 8; i++)
                {
                    if (servoDir[i] == 1)
                    {
                        eeServoDir |= (Byte)(1 << i);
                    }
                }
                eepromData[Eeprom_Model_Offset() + 103] = eeServoDir;
            }
        }

        public Byte[] MC_FunctionToServoTable
        {
            get
            {
                Byte[] ftos = new Byte[8];
                for (int i = 0; i < 8; i++)
                {
                    ftos[i] = (Byte)eepromData[Eeprom_Model_Offset() + 104 + i];
                }

                return ftos;
            }
            set
            {
                for (int i = 0; i < 8; i++)
                {
                    eepromData[Eeprom_Model_Offset() + 104 + i] = value[i];
                }
            }
        }

        public Byte[] MC_ServoToFunctionTable
        {
            get
            {

                Byte[] ftos = MC_FunctionToServoTable;
                Byte[] stof = new Byte[8];
                for (int i = 0; i < 8; i++)
                {
                    int s = ftos[i];
                    stof[s] = (Byte)i;
                }

                return stof;
            }
        }


        public Byte MC_servoMapping
        {
            get
            {
                Byte[] ftos = MC_FunctionToServoTable;
                if (ftos[0] == 0)
                {
                    // Futaba
                    return 0;
                }
                else
                {
                    // Spektrum
                    return 1;
                }

            }
            set
            {
                Byte[] ftos = MC_FunctionToServoTable;
                if (value == 0)
                {
                    //MIX_OUT_AILERON = 0x30,
                    //MIX_OUT_THROTTLE,
                    //MIX_OUT_ELEVATOR,
                    //MIX_OUT_RUDDER,
                    //MIX_OUT_GYRO_GAIN,
                    //MIX_OUT_PITCH,
                    //MIX_OUT_AUX1,
                    //MIX_OUT_AUX2,

                    // Futaba
                    ftos[0] = 0;
                    ftos[1] = 2;
                    ftos[2] = 1;
                    ftos[3] = 3;
                    ftos[4] = 4;
                    ftos[5] = 5;
                    ftos[6] = 6;
                    ftos[7] = 7;
                }
                else
                {
                    // Spektrum
                    ftos[0] = 1;
                    ftos[1] = 0;
                    ftos[2] = 2;
                    ftos[3] = 3;
                    ftos[4] = 4;
                    ftos[5] = 5;
                    ftos[6] = 6;
                    ftos[7] = 7;
                }
                MC_FunctionToServoTable = ftos;
            }
        }

        public SByte[][] MC_endPoints
        {
            get
            {
                SByte[][] ep = new SByte[2][];
                for (int a = 0; a < 2; a++)
                {
                    ep[a] = new SByte[8];
                }

                for (int a = 0; a < 2; a++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        ep[a][i] = (SByte)eepromData[Eeprom_Model_Offset() + 165 + a*8 + i];
                    }
                }

                return ep;
            }
            set
            {
                for (int a = 0; a < 2; a++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        eepromData[Eeprom_Model_Offset() + 165 + a*8 + i] = (Byte)value[a][i];
                    }
                }
            }
        }

        public SByte[][] MC_expo
        {
            get
            {
                SByte[][] expo = new SByte[4][];
                for (int i = 0; i < 4; i++)
                {
                    expo[i] = new SByte[2];
                }

                for (int i = 0; i < 4; i++)
                {
                    for (int m = 0; m < 2; m++)
                    {
                        expo[i][m] = (SByte)eepromData[Eeprom_Model_Offset() + 152 + i * 2 + m];
                    }
                }

                return expo;
            }
            set
            {
                for (int i = 0; i < 4; i++)
                {
                    for (int m = 0; m < 2; m++)
                    {
                        eepromData[Eeprom_Model_Offset() + 152 + i * 2 + m] = (Byte)value[i][m];
                    }
                }
            }
        }

        // Containers
        Byte[] eepromData = new Byte[2048];

        public Eeprom(string fileName)
        {
            FileStream inFile;

            // Load the file
            inFile = new FileStream(fileName, FileMode.Open, FileAccess.Read);
            inFile.Read(eepromData, 0, 2048);
            inFile.Close();

        }
    }
}
