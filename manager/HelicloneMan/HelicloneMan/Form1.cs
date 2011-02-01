using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;

namespace HelicloneMan
{
    public partial class frmMain : Form
    {
        Eeprom loadedEeprom = null;

        public frmMain()
        {
            InitializeComponent();
        }

        private string FunctionToText(int f)
        {
            switch (f)
            {
                case 0:
                    return "AIL";
                case 1:
                    return "THR";
                case 2: 
                    return "ELE";
                case 3:
                    return "RUD";
                case 4:
                    return "GYR";
                case 5:
                    return "PIT";
                case 6:
                    return "AUX1";
                case 7:
                    return "AUX2";
                default:
                    return "---";
            }
        }

        public void LoadRadioConfig()
        {
            if (loadedEeprom != null)
            {
                udContrast.Value = loadedEeprom.RC_Contrast;
                cbBacklight.SelectedIndex = loadedEeprom.RC_Backlight;
                cbKeyBeep.SelectedIndex = loadedEeprom.RC_KeyBeep;
                cbBeepAlarm.SelectedIndex = loadedEeprom.RC_AlarmBeep;
                udMboxTimeout.Value = loadedEeprom.RC_Message_box_timeout;
                udVoltageWarning.Value = loadedEeprom.RC_VoltageWarning;

                // ADC_C
                UInt16[][] adc_c = loadedEeprom.RC_adc_c;
                udAdc_C_0_0.Value = adc_c[0][0];
                udAdc_C_0_1.Value = adc_c[0][1];
                udAdc_C_0_2.Value = adc_c[0][2];

                udAdc_C_1_0.Value = adc_c[1][0];
                udAdc_C_1_1.Value = adc_c[1][1];
                udAdc_C_1_2.Value = adc_c[1][2];

                udAdc_C_2_0.Value = adc_c[2][0];
                udAdc_C_2_1.Value = adc_c[2][1];
                udAdc_C_2_2.Value = adc_c[2][2];

                udAdc_C_3_0.Value = adc_c[3][0];
                udAdc_C_3_1.Value = adc_c[3][1];
                udAdc_C_3_2.Value = adc_c[3][2];

                udAdc_C_4_0.Value = adc_c[4][0];
                udAdc_C_4_1.Value = adc_c[4][1];
                udAdc_C_4_2.Value = adc_c[4][2];

                udAdc_C_5_0.Value = adc_c[5][0];
                udAdc_C_5_1.Value = adc_c[5][1];
                udAdc_C_5_2.Value = adc_c[5][2];

                udAdc_C_6_0.Value = adc_c[6][0];
                udAdc_C_6_1.Value = adc_c[6][1];
                udAdc_C_6_2.Value = adc_c[6][2];

                udAdc_C_7_0.Value = adc_c[7][0];
                udAdc_C_7_1.Value = adc_c[7][1];
                udAdc_C_7_2.Value = adc_c[7][2];

                cbSelectedModel.SelectedIndex = loadedEeprom.RC_SelectedModel;

                // Current model...
                tbM_Name.Text = loadedEeprom.MC_Name;
                cbM_Type.SelectedIndex = loadedEeprom.MC_ModelType;

                SByte[] trim = loadedEeprom.MC_trim;
                udM_trim_0.Value = trim[0];
                udM_trim_1.Value = trim[1];
                udM_trim_2.Value = trim[2];
                udM_trim_3.Value = trim[3];

                SByte[] subTrim = loadedEeprom.MC_subTrim;
                udSubtrim0.Value = subTrim[0];
                udSubtrim1.Value = subTrim[1];
                udSubtrim2.Value = subTrim[2];
                udSubtrim3.Value = subTrim[3];
                udSubtrim4.Value = subTrim[4];
                udSubtrim5.Value = subTrim[5];
                udSubtrim6.Value = subTrim[6];
                udSubtrim7.Value = subTrim[7];

                Byte[] servoDir = loadedEeprom.MC_servoDirection;
                cbServoDir0.SelectedIndex = servoDir[0];
                cbServoDir1.SelectedIndex = servoDir[1];
                cbServoDir2.SelectedIndex = servoDir[2];
                cbServoDir3.SelectedIndex = servoDir[3];
                cbServoDir4.SelectedIndex = servoDir[4];
                cbServoDir5.SelectedIndex = servoDir[5];
                cbServoDir6.SelectedIndex = servoDir[6];
                cbServoDir7.SelectedIndex = servoDir[7];

                cbServoMap.SelectedIndex = loadedEeprom.MC_servoMapping;
                Byte[] stof = loadedEeprom.MC_ServoToFunctionTable;
                lCh0.Text = "CH1 (" + FunctionToText(stof[0]) + ")";
                lCh1.Text = "CH2 (" + FunctionToText(stof[1]) + ")";
                lCh2.Text = "CH3 (" + FunctionToText(stof[2]) + ")";
                lCh3.Text = "CH4 (" + FunctionToText(stof[3]) + ")";
                lCh4.Text = "CH5 (" + FunctionToText(stof[4]) + ")";
                lCh5.Text = "CH6 (" + FunctionToText(stof[5]) + ")";
                lCh6.Text = "CH7 (" + FunctionToText(stof[6]) + ")";
                lCh7.Text = "CH8 (" + FunctionToText(stof[7]) + ")";

                SByte[][] ep = loadedEeprom.MC_endPoints;
                udEpL0.Value = ep[0][0];
                udEpL1.Value = ep[0][1];
                udEpL2.Value = ep[0][2];
                udEpL3.Value = ep[0][3];
                udEpL4.Value = ep[0][4];
                udEpL5.Value = ep[0][5];
                udEpL6.Value = ep[0][6];
                udEpL7.Value = ep[0][7];

                udEpH0.Value = ep[1][0];
                udEpH1.Value = ep[1][1];
                udEpH2.Value = ep[1][2];
                udEpH3.Value = ep[1][3];
                udEpH4.Value = ep[1][4];
                udEpH5.Value = ep[1][5];
                udEpH6.Value = ep[1][6];
                udEpH7.Value = ep[1][7];


                SByte[][] expo = loadedEeprom.MC_expo;
                udM_expo_N_0.Value = expo[0][0];
                udM_expo_N_1.Value = expo[1][0];
                udM_expo_N_2.Value = expo[2][0];
                udM_expo_N_3.Value = expo[3][0];

                udM_expo_DR_0.Value = expo[0][1];
                udM_expo_DR_1.Value = expo[1][1];
                udM_expo_DR_2.Value = expo[2][1];
                udM_expo_DR_3.Value = expo[3][1];

                SByte[] swash = loadedEeprom.MC_SwashThrow;
                udSwash0.Value = swash[0];
                udSwash1.Value = swash[1];
                udSwash2.Value = swash[2];

                SByte[] gyro = loadedEeprom.MC_GyroGain;
                udGyro0.Value = gyro[0];
                udGyro1.Value = gyro[1];

                // Select one curve...
                cbSelectedCurve.SelectedIndex = 0;
                SByte[][] curves = loadedEeprom.MC_curves;
                udCurve0.Value = curves[cbSelectedCurve.SelectedIndex][0];
                udCurve1.Value = curves[cbSelectedCurve.SelectedIndex][1];
                udCurve2.Value = curves[cbSelectedCurve.SelectedIndex][2];
                udCurve3.Value = curves[cbSelectedCurve.SelectedIndex][3];
                udCurve4.Value = curves[cbSelectedCurve.SelectedIndex][4];

            }
        }

        //
        // For execute
        //
        public delegate void UpdateLogTextCallback(string text);
        private void UpdateLogText(string text)
        {
            // Set the textbox text.
            tbLog.Text = tbLog.Text + text;
        }

        public delegate void UpdateLogCharCallback(char oneChar);
        private void UpdateLogChar(char oneChar)
        {
            // Set the textbox text.
            tbLog.Text = tbLog.Text + (char)oneChar;
        }

        private string cmd_path;
        private string cmd_command;
        private string cmd_args;
        private bool cmd_ok;

        private void ExecuteCommand()
        {
            try
            {
                System.Diagnostics.ProcessStartInfo procStartInfo =
                    new System.Diagnostics.ProcessStartInfo();

                procStartInfo.FileName = cmd_path + "\\" + cmd_command;
                procStartInfo.Arguments = cmd_args;
                procStartInfo.WorkingDirectory = cmd_path;

                // The following commands are needed to redirect the standard output.
                // This means that it will be redirected to the Process.StandardOutput StreamReader.
                procStartInfo.RedirectStandardError = true;
                procStartInfo.UseShellExecute = false;
                
                // Do not create the black window.
                procStartInfo.CreateNoWindow = true;
                
                // Now we create a process, assign its ProcessStartInfo and start it
                System.Diagnostics.Process proc = new System.Diagnostics.Process();
                proc.StartInfo = procStartInfo;
                proc.Start();

                // Get the output into a string
                while (!proc.StandardError.EndOfStream)
                {
                    char oneChar = (char)proc.StandardError.Read();
                    tbLog.Invoke(new UpdateLogCharCallback(this.UpdateLogChar), new object[]{oneChar});
                }

                //proc.WaitForExit();
                cmd_ok = true;
            }
            catch (Exception objException)
            {
                // Log the exception
                tbLog.Invoke(new UpdateLogTextCallback(this.UpdateLogText), new object[] { objException.Message });
                cmd_ok = false;
            }
        }

        private void btnReadEeprom_Click(object sender, EventArgs e)
        {
            saveFileDlg.DefaultExt = "bin";
            saveFileDlg.Filter = "Bin file (*.bin)|*.bin|All files (*.*)|*.*";
            DialogResult res = saveFileDlg.ShowDialog();

            if (res == DialogResult.OK)
            {
                tbLog.Clear();
                cmd_path = Directory.GetCurrentDirectory() + "\\avrdude";
                cmd_command = "avrdude.exe";
                cmd_args = "-c " + Properties.Settings.Default.AvrDude_Device + " -p m64 -U eeprom:r:" + saveFileDlg.FileName + ":r";

                tbLog.Text = "EXEC: " + cmd_path + "\\" + cmd_command + " " + cmd_args + "\r\n";
                ExecuteCommand();
                if (cmd_ok)
                {
                    if (tbLog.Text.Contains("error"))
                    {
                        MessageBox.Show("Could not read from transmitter...", "ERROR!!!");
                    }
                    else
                    {
                        loadedEeprom = new Eeprom(saveFileDlg.FileName);
                        LoadRadioConfig();
                    }
                }
                else
                {
                    MessageBox.Show("Could not read from transmitter...", "ERROR!!!");
                    loadedEeprom = null;
                }
            }
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void btnReadFlash_Click(object sender, EventArgs e)
        {
            saveFileDlg.DefaultExt = "hex";
            saveFileDlg.Filter = "Hex file (*.hex)|*.hex|All files (*.*)|*.*";
            DialogResult res = saveFileDlg.ShowDialog();

            if (res == DialogResult.OK)
            {
                tbLog.Clear();
                cmd_path = Directory.GetCurrentDirectory() + "\\avrdude";
                cmd_command = "avrdude.exe";
                cmd_args = "-c " + Properties.Settings.Default.AvrDude_Device + " -p m64 -U flash:r:" + saveFileDlg.FileName + ":i";

                tbLog.Text = "EXEC: " + cmd_path + "\\" + cmd_command + " " + cmd_args + "\r\n";
                Thread myThread = new Thread(ExecuteCommand);
                myThread.Start();
            }
        }

        private void cbSelectedModel_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Whats to check other model...

            // Check if model is OK...
            loadedEeprom.RC_SelectedModel = (Byte)cbSelectedModel.SelectedIndex;
            if (loadedEeprom.MC_Name == "")
            {
                MessageBox.Show("Not a valid model here!");
                loadedEeprom.RC_SelectedModel = 0;
            }
            LoadRadioConfig();
        }

        private void cbServoMap_SelectedIndexChanged(object sender, EventArgs e)
        {
            // We need to regenerate the servo map & texts...
            loadedEeprom.MC_servoMapping = (Byte)cbServoMap.SelectedIndex;

            Byte[] stof = loadedEeprom.MC_ServoToFunctionTable;
            lCh0.Text = "CH1 (" + FunctionToText(stof[0]) + ")";
            lCh1.Text = "CH2 (" + FunctionToText(stof[1]) + ")";
            lCh2.Text = "CH3 (" + FunctionToText(stof[2]) + ")";
            lCh3.Text = "CH4 (" + FunctionToText(stof[3]) + ")";
            lCh4.Text = "CH5 (" + FunctionToText(stof[4]) + ")";
            lCh5.Text = "CH6 (" + FunctionToText(stof[5]) + ")";
            lCh6.Text = "CH7 (" + FunctionToText(stof[6]) + ")";
            lCh7.Text = "CH8 (" + FunctionToText(stof[7]) + ")";

        }

        private void btnLoadEeprom_Click(object sender, EventArgs e)
        {
            openFileDlg.DefaultExt = "bin";
            openFileDlg.Filter = "Bin file (*.bin)|*.bin|All files (*.*)|*.*";
            DialogResult res = openFileDlg.ShowDialog();

            if (res == DialogResult.OK)
            {
                tbLog.Clear();

                loadedEeprom = new Eeprom(openFileDlg.FileName);
                LoadRadioConfig();
                tbLog.Text = "Loaded Eeprom from: " + openFileDlg.FileName;
            }
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            // Draw the cross
            Pen penCurrent = new Pen(Color.Blue);
            penCurrent.Width = 2;
            //Point[] line = { new Point(0,0), new Point(100,100)};

            // DRAW X-AXIS
            e.Graphics.DrawLine(penCurrent, new Point(0, curvePanel.Height/2), new Point(curvePanel.Width, curvePanel.Height / 2));

            // DRAW Y-AXIS
            e.Graphics.DrawLine(penCurrent, new Point(curvePanel.Width/2, 0), new Point(curvePanel.Width/2, curvePanel.Height));

            Point x1y1, x2y2;
            float dy,dx;
            float scale = 0.96f;

            penCurrent = new Pen(Color.Black);
            penCurrent.Width = 2;
            
            // 1st segment...
            dx = ((float)0.5f*curvePanel.Width)*-100.0f/100.0f;
            dy = ((float)0.5f*curvePanel.Height*(float)(udCurve0.Value))/100.0f;
            dx *= scale;
            dy *= scale;
            x1y1 = new Point(curvePanel.Width/2 + (int)dx, curvePanel.Height/2 - (int)dy);

            dx = ((float)0.5f*curvePanel.Width)*-50.0f/100.0f;
            dy = ((float)0.5f*curvePanel.Height*(float)(udCurve1.Value))/100.0f;
            dx *= scale;
            dy *= scale;
            x2y2 = new Point(curvePanel.Width / 2 + (int)dx, curvePanel.Height / 2 - (int)dy);
            e.Graphics.DrawLine(penCurrent, x1y1, x2y2);

            // 2nd segment
            x1y1 = x2y2;

            dx = ((float)0.5f * curvePanel.Width) * 0.0f / 100.0f;
            dy = ((float)0.5f * curvePanel.Height * (float)(udCurve2.Value)) / 100.0f;
            dx *= scale;
            dy *= scale;
            x2y2 = new Point(curvePanel.Width / 2 + (int)dx, curvePanel.Height / 2 - (int)dy);
            e.Graphics.DrawLine(penCurrent, x1y1, x2y2);

            // 3rd segment
            x1y1 = x2y2;

            dx = ((float)0.5f * curvePanel.Width) * 50.0f / 100.0f;
            dy = ((float)0.5f * curvePanel.Height * (float)(udCurve3.Value)) / 100.0f;
            dx *= scale;
            dy *= scale;
            x2y2 = new Point(curvePanel.Width / 2 + (int)dx, curvePanel.Height / 2 - (int)dy);
            e.Graphics.DrawLine(penCurrent, x1y1, x2y2);

            // 4th segment
            x1y1 = x2y2;

            dx = ((float)0.5f * curvePanel.Width) * 100.0f / 100.0f;
            dy = ((float)0.5f * curvePanel.Height * (float)(udCurve4.Value)) / 100.0f;
            dx *= scale;
            dy *= scale;
            x2y2 = new Point(curvePanel.Width / 2 + (int)dx, curvePanel.Height / 2 - (int)dy);
            e.Graphics.DrawLine(penCurrent, x1y1, x2y2);
            
        }

        private void anyCurve_ValueChanged(object sender, EventArgs e)
        {
            curvePanel.Invalidate();
        }

        private void cbSelectedCurve_SelectedIndexChanged(object sender, EventArgs e)
        {
            SByte[][] curves = loadedEeprom.MC_curves;

            udCurve0.Value = curves[cbSelectedCurve.SelectedIndex][0];
            udCurve1.Value = curves[cbSelectedCurve.SelectedIndex][1];
            udCurve2.Value = curves[cbSelectedCurve.SelectedIndex][2];
            udCurve3.Value = curves[cbSelectedCurve.SelectedIndex][3];
            udCurve4.Value = curves[cbSelectedCurve.SelectedIndex][4];

        }
    }
}
