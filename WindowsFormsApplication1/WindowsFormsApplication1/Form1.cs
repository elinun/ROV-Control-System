using System;
using System.IO.Ports;
using System.Threading;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        SerialPort port;
        TcpListener net;
        Thread readThread;
        Thread netThread;

        public Form1()
        {
            port = new SerialPort();
            readThread = new Thread(Read);
            netThread = new Thread(DoNetStuff);
            net = new TcpListener(1738);
            //net.Start();
            netThread.Start();
            InitializeComponent();
            portSelector.Items.Add("TEST");
            foreach(var cur in SerialPort.GetPortNames())
            {
                portSelector.Items.Add(cur);
            }
            this.KeyPreview = true;
            this.KeyPress += new KeyPressEventHandler(mKeyPress);
            this.MouseMove += new MouseEventHandler(mMouseMove);
            this.MouseWheel += new MouseEventHandler(mMouseWheel);
            this.FormClosing += new FormClosingEventHandler(mOnClose);

        }

        void mOnClose(object sender, FormClosingEventArgs e)
        {
            output.Text = "Closing window";
            net.Stop();
            port.Close();
        }

        void DoNetStuff()
        {
            TcpClient s = default(TcpClient);
            net.Start();
            while (true)
            {
                s = net.AcceptTcpClient() ;
                
                NetworkStream stream = s.GetStream();
                while (s.Connected)
                {
                    byte[] buff = new byte[5];
                    stream.Read(buff, 0, buff.Length);
                    string x = Encoding.ASCII.GetString(buff, 0, buff.Length);
                    //string pwr = x.Substring(1, x.Length - 1);
                    //x = x.Substring(0, 1);
                    //int power = Int32.Parse(pwr);
                    // power /= 20;
                    //output.Text = x+power;//actually angle for servo
                    if (port.IsOpen)
                    {
                        port.Write(x);
                    }

                }
            }



            //accepting first connection
            
            //output.Text = "Net Connection Accepted";
           /* NetworkStream stream = s.GetStream();
            byte[] buff = new byte[4];
            while(stream.Read(buff, 0, 4) != -1)
            {
                String x = System.Text.Encoding.UTF8.GetString(buff);
                String pwr = x.Substring(1, x.Length - 1);
                x = x.Substring(0, 1);
                int power = Int32.Parse(pwr);
                power /= 20;
                //output.Text = x+power;//actually angle for servo
                if (port.IsOpen)
                {
                    port.Write(x+power);
                }
            }*/
           // output.Text = "Net Connection closed";
        }

        void mMouseMove(object sender, MouseEventArgs e)
        {
            if (port.IsOpen)
            {
                port.Write("f" + e.X);
                port.Write("l" + e.Y);
            }

        }

        void mMouseWheel(object sender, MouseEventArgs e)
        {
            output.Text = e.Delta+"";
            port.Write("u" + (127+e.Delta));
        }

        void mKeyPress(object sender, KeyPressEventArgs e)
        {
            if (port.IsOpen)
            {
                port.Write(e.KeyChar.ToString());
            }
        }

        private void Read()
        {
            while (port.IsOpen)
            {
                try {
                    output.Text = port.ReadLine();
                }catch(TimeoutException e)
                {

                }
            }
        }

        private void connectBtn_Click(object sender, EventArgs e)
        {
            //set all properties
            port.Parity = Parity.None;
            port.DataBits = 8;
            port.StopBits = StopBits.One;
            port.PortName = portSelector.SelectedItem.ToString();
            port.BaudRate = 9600;
            port.ReadTimeout = 500;
            port.WriteTimeout = 500;
            Console.WriteLine("Button clicked");
            port.Open();
            readThread.Start();
            //new Controller(port);
            output.Text = "Connected";
        }
    }
}
