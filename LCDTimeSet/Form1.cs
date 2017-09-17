using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LCDTimeSet
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            comboBox1.Items.Clear();
            string[] serialPortNames = SerialPort.GetPortNames();
            foreach (string portName in serialPortNames)
            {
                comboBox1.Items.Add(portName);
            }

            timer1.Start();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            DateTime dt = DateTime.Now;
            textBox1.Text = dt.ToString("G");
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (comboBox1.Text.Length == 0)
            {
                MessageBox.Show("COM 포트를 선택하세요.", "포트선택");
                comboBox1.Focus();
                return;
            }

            // 시리얼포트 초기화
            serialPort1.PortName = comboBox1.Text;
            serialPort1.BaudRate = 9600;
            serialPort1.Parity = Parity.None;
            serialPort1.DataBits = 8;
            serialPort1.StopBits = StopBits.One;
            serialPort1.Handshake = Handshake.None;

            try
            {
                serialPort1.Open();

                // 현재시각
                DateTime dt = DateTime.Now;

                // 전송에 소요되는 시간이 있으므로 +2초 보정한다.
                dt = dt.AddSeconds(2);

                // 현재시각을 시간문자열로 변경
                string timeText = dt.ToString("yyyyMMddHHmmss");

                string sendStr = "T" + timeText + ".";

                // 전송처리
                serialPort1.Write(sendStr);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Exception-" + ex.ToString(), "전송오류");
                return;
            }
            finally
            {
                serialPort1.Close();
            }

            MessageBox.Show("전송 완료!", "OK");
        }
    }
}
