using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WindowsFormsApplication1
{
    class Settings
    {
       /* public char fwdKey = 'w';
        public char bkdKey = 's';
        public char lftKey = 'a';
        public char rgtKey = 'd';
        public char upKey = 'u';
        public char dwnKey = 'j'; replaced with mouse*/
        public char camUp = 'i';//replace cam mvmt with vr tilt
        public char camDwn = 'k';
        public char clawOpen = ',';
        readonly public char clawClose = '.';
        /*public char clkWise = 'x';//rotate ROV clockWise
        public char cntrClk = 'z';//counterClock wise replaced with mouse*/
        
        public Settings()
        {

        }
    }
}
