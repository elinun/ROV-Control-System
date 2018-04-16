using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using SlimDX.DirectInput;
using System.Net.Sockets;
using System.Linq;

namespace WindowsFormsApp1
{
    public partial class Form1 : Form
    {
        TcpClient tcpclnt;
        Stream stream;
        DirectInput input = new DirectInput();
        SlimDX.DirectInput.Joystick stick;
        static Joystick[] sticks;
        Thread pollThread;
        int x = 0;
        int y = 0;
        int z = 0;
        char divider = '\t';

        public void poll()
        {
            while (true)
            {
                if (stick != null)
                {
                    stickHandle(stick, 0);
                }
                else
                {
                    sticks = GetSticks();
                }
                Thread.Sleep(15);
            }
        }

        public Joystick[] GetSticks()
        {
            List<SlimDX.DirectInput.Joystick> sticks = new List<SlimDX.DirectInput.Joystick>();
           // List<Microsoft.DirectX.DirectInput.Joystick> mSticks = new List<Microsoft.DirectX.DirectInput.Joystick>();
            foreach (DeviceInstance device in input.GetDevices(DeviceClass.GameController, DeviceEnumerationFlags.AttachedOnly))
            {
                try
                {
                    stick = new SlimDX.DirectInput.Joystick(input, device.InstanceGuid);
                    stick.Acquire();
                    sticks.Add(stick);
                    Invoke(new MethodInvoker(delegate
                    {
                        debug.Text = "Controller Connected.";
                        /*foreach (bool btn in buttons) {
                            controlOut.Text += (btn ? "t " : "f ") ;
                        }*/

                    }));
                }
                catch (Exception e) { debug.Text += e.ToString(); }
            }
            return sticks.ToArray();
        }

        int lowDeadBand = 1100;//really min
        int highDeadBand = 1900;//really max
        int servoPos = 90;//temporary

        void stickHandle(Joystick stick, int id)
        {
            JoystickState state = new JoystickState();
            state = stick.GetCurrentState();
            x = state.X;
            y = state.Y;
            z = state.Z;
            int zPwr = (z / 82)+1100;//these are NOT arbitrarily created numbers
            int xPwr = (x / 82)+1100;
            int yPwr = (y / 82)+1100;
            int rightTrig = (state.RotationY / 164) + 1500;
            int leftTrig = 1500-(state.RotationX / 164);
            bool[] buttons = state.GetButtons();

            Invoke(new MethodInvoker(delegate
                {
                    debug.Text = "X: "+xPwr;                   
                }));
                
                if (tcpclnt != null)
                {
                    if (tcpclnt.Connected)
                    {
                    String str = ((yPwr > 1550 && yPwr<highDeadBand)? divider+"f" + yPwr : "") + (yPwr < 1450 && yPwr>lowDeadBand ? divider+"f" + yPwr : "") +
                        (xPwr > 1550 && xPwr <highDeadBand ? divider+"l" + xPwr : "") + (xPwr < 1450 && xPwr>lowDeadBand ? divider+"l" + xPwr : "") +
                        (buttons[3] ? divider+"c0001" : "") + (buttons[1] ? divider+"v0001" : "") +
                        (buttons[10] ? divider+"r0000":"")+
                        (buttons[4] ? divider+"x0100":"")+(buttons[5] ? divider+"x-100":"")+
                        (buttons[2] ? divider+",0001" : "") + (buttons[0] ? divider+".0001" : "") +//servo, adding this many degrees to position
                        (zPwr > 1550 || zPwr < 1450 ? divider+"z" + zPwr : "")  +
                        checkForBuzzer(buttons[13])+
                        (rightTrig > 1501 ? divider+"h" + rightTrig : "") + (leftTrig < 1499 ? divider+"h" + leftTrig : "");
                        str += checkForStop(xPwr, yPwr, zPwr);
                    if (str.Length > 0)
                    {
                        
                        str+=divider+"\n";
                        ASCIIEncoding asen = new ASCIIEncoding();
                        Invoke(new MethodInvoker(delegate
                        {
                            //debug.Text = asen.GetString(asen.GetBytes(str));
                        }));
                        byte[] ba = asen.GetBytes(str);
                        stream.Write(ba, 0, ba.Length);
                    }
                       
                    }
                
            }
           
        }
        string tempClawControl(string cmd)
        {
            return cmd;
        }

        bool xStopped = true;
        bool yStopped = true;
        bool zStopped = true;
        bool buzzerOn = false;

        string checkForBuzzer(bool button)
        {
            string retVal = "";
            if (!buzzerOn && button)
            {
                retVal += divider+"a0000";
                buzzerOn = true;
            }
            else if (buzzerOn && !button)
            {
                buzzerOn = false;
                retVal += divider + "a0000";
            }

                return retVal;
        }

        string checkForStop(int x, int y, int z)
        {
            string retVal = "";
            if(x<1525 && x > 1475 && !xStopped)
            {
                retVal += divider+"l1500";
            }
            xStopped = !(x > 1525 || x < 1475);

            if (y < 1525 && y > 1475 && !yStopped)
            {
                retVal += divider + "f1500";
            }
            yStopped = !(y > 1525 || y < 1475);

            if (z < 1525 && z > 1475 && !zStopped)
            {
                retVal += divider + "z1500";
            }
            zStopped = !(z > 1525 || z < 1475);
            return retVal;
        }

        public Form1()
        {
            InitializeComponent();
            //GetSticks();
            sticks = GetSticks();
            pollThread = new Thread(poll);
            pollThread.Start();

            this.FormClosing += monClosing;
            ConnectBtn.Click += ConnectBtn_Click;
           // this.MouseMove += mouseMove;
        }

          

            public void monClosing(object ev, FormClosingEventArgs e)// a better cleanup
        {
            if (pollThread.IsAlive)
            {
                //stick.Dispose();
                pollThread.Abort();
            }
            if(tcpclnt != null)
            {
                tcpclnt.Close();
            }
        }

        private void ConnectBtn_Click(object sender, EventArgs e)
        {
            debug.Text = "Connected to ROV";
            if (tcpclnt != null)
            {
                tcpclnt.Close();
            }
            try
            {
                tcpclnt = new TcpClient();
                tcpclnt.SendTimeout = 500;

                tcpclnt.Connect(arduinoIP.Text, 1740);
                stream = tcpclnt.GetStream();
                stream.WriteTimeout = 500;
            }
            catch(Exception ex) { debug.Text = "Network Error"; }
        }


        int seconds = 0;
        private void timer1_Tick(object sender, EventArgs e)
        {
            seconds++;
            timer.Text = addZeros(seconds/60, 2)+":"+addZeros(seconds%60, 2);
        }

        string addZeros(int num, int numOfDigits)
        {
            string ret = "";
            for(int i = numOfDigits-1; i>-1; i--)
            {
                if ((num / (int)(Math.Pow(10, i))) == 0)
                {
                    ret += "0";
                }
                else
                {
                    ret += num.ToString();
                    i = -1;
                }
            }
            return ret;
        }

        private void button1_Click(object sender, EventArgs e)//Start/Stop
        {
            if (timer1.Enabled)
            {
                button1.Text = "Start";
                timer1.Stop();
            }
            else
            {
                button1.Text = "Stop";
                timer1.Interval = 1000;
                timer1.Start();
            }
        }

        private void button2_Click(object sender, EventArgs e)//reset
        {
            seconds = 0;
            timer.Text = "00:00";
        }

        private void vScrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            string name = ((ScrollBar)sender).Name;
            string str = "";
            if (tcpclnt == null)
            {
                if ( stream == null)
                {
                    str = divider+"t" + makeTrimValue(e.NewValue) + name.Substring(name.Length - 1)+divider;
                   
                   
                    ASCIIEncoding asen = new ASCIIEncoding();
                    Invoke(new MethodInvoker(delegate
                    {
                        debug.Text = asen.GetString(asen.GetBytes(str));
                    }));
                    byte[] ba = asen.GetBytes(str);
                    //stream.Write(ba, 0, ba.Length);
                }
            }
        }

        string makeTrimValue(int pwr)
        {
            string ret = "";
            if (pwr > 0)
            {
                ret += addZeros(pwr, 3);
            }
            else
            {
                ret += "-";
                ret += addZeros(Math.Abs(pwr), 2);
            }
            return ret;
        }

        bool saving = false;
        private void button3_Click(object sender, EventArgs e)//save
        {
            saving = true;
            saveFileDialog1.Filter = "Text File|*.txt";
            saveFileDialog1.FileName = "trims.txt";
            saveFileDialog1.ShowDialog();
        }

        private void button4_Click(object sender, EventArgs e)//load
        {
            saving = false;
            saveFileDialog1.OverwritePrompt = false;
            saveFileDialog1.ShowDialog();
        }

        private void saveFileDialog1_FileOk(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (saving)
            {
                string str = "";
                ScrollBar bar;
                for (int i = 1; i < 7;i++)
                {
                    bar = this.Controls.Find("vScrollBar"+i, true).FirstOrDefault() as ScrollBar;
                    str += bar.Value+",";
                    
                }
                str = str.Substring(0, str.Length - 1);
                using (StreamWriter sw = new StreamWriter(saveFileDialog1.FileName))
                    sw.WriteLine(str);


            }
            else
            {
                String trims;
                using (StreamReader sr = new StreamReader(saveFileDialog1.FileName))
                    trims = sr.ReadLine();
                string[] sValues = trims.Split(',');
                ScrollBar bar;
                for(int i = 1; i < 7; i++)
                {
                    bar = this.Controls.Find("vScrollBar" + i, true).FirstOrDefault() as ScrollBar;
                    bar.Value = int.Parse(sValues[i - 1]);
                    vScrollBar_Scroll((object)bar, new ScrollEventArgs(ScrollEventType.SmallIncrement, bar.Value));
                }
            }
        }

 
    }
}