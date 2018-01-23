using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.Windows.Input;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Controller : Form
    {
        public SerialPort port;
        public Controller(SerialPort port)
        {
            InitializeComponent();
            this.port = port;
            this.KeyPreview = true;
            this.KeyPress += new KeyPressEventHandler(mKeyPress);
            this.MouseMove += new MouseEventHandler(mMouseMove);
            
        }

        void mMouseMove(object sender, MouseEventArgs e)
        {
            if (port.IsOpen)
            {
                port.Write("f" + e.X);
                port.Write("l"+e.Y);
                if(e.Button == MouseButtons.Left)
                {
                    port.Write("d123");
                }
            }
            
        }

        void mKeyPress(object sender, KeyPressEventArgs e)
        {
            if (port.IsOpen) {
                port.Write(e.KeyChar.ToString());
         }
        }
    }
}
