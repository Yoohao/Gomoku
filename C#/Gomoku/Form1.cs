using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.IO;

namespace Gomoku
{
    public partial class Form1 : Form
    {
        Label[,] chess;
        int[,] Board;
        Image black = Resource1.black;
        Image white = Resource1.white;

        bool turn, You, Internet;

        TcpClient tcp;
        NetworkStream Stream;
        StreamReader SR;
        StreamWriter SW;

        public Form1()
        {
            turn = true;
            Internet = false;
 
            InitializeComponent();
                
            chess = new Label[10, 10];
            Board = new int[10, 10];
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    chess[i, j] = new Label();
                    chess[i, j].Name = i.ToString() + j.ToString();
                    chess[i, j].Width = 35;
                    chess[i, j].Height = 35;
                    chess[i, j].Location = new Point(36 + 44 * i, 36 + 44 * j);
                    chess[i, j].BackColor = Color.Transparent;
                    chess[i, j].Click += new EventHandler(LB_Click);
                    pictureBox1.Controls.Add(chess[i, j]);
                }
            }
            toolStripStatusLabel1.Text = "Disconnected";
            label1.Text = "Turn to: " + (turn ? "Black" : "White") + " Side";
        }

        private void LB_Click(object sender, EventArgs e)
        {
            if (You != turn && Internet)
                return;
            Label tmp = (Label)sender;
            int x, y;
            x = int.Parse(tmp.Name.Substring(0, 1));
            y = int.Parse(tmp.Name.Substring(1, 1));
            if (Board[x, y] != 0)
                return;
            if (turn)
            {
                Board[x, y] = 1;
                tmp.Image = black;
            }
            else
            {
                Board[x, y] = 2;
                tmp.Image = white;
            }

            if (Internet)
            {
                SW.WriteLine("/gomoku " + x.ToString() + y.ToString() + "\r\n");
                SW.Flush();
            }
            richTextBox1.AppendText((turn ? "Black" : "White") + " side place chess piece at (" 
                                + (char)(x + 'A') + ", " + (char)(y + '0') + ")\n");
            toolStripProgressBar1.Value --;
            if (toolStripProgressBar1.Value == 0)
            {
                MessageBox.Show("Draw", "Message", MessageBoxButtons.OK, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1);
                InitBoard();
                return;
            }
            if (referee(x, y, turn))
                return;
            turn = !turn;
            label1.Text = "Turn to: " + (turn ? "Black" : "White") + " Side";
        }

        private bool referee(int x, int y, bool who)
        {
            int[] dir = new int[8];
            int obj = (who ? 1 : 2);
            for (int i = 0; i < 8; i++)
                dir[i] = 0;

            for (int i = 1; i <= 4; i++)
            {
                if (y - i >= 0 && Board[x, y-i] == obj)
                    dir[0]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (x + i < 10 && Board[x + i, y] == obj)
                    dir[2]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (y + i < 10 && Board[x, y + i] == obj)
                    dir[4]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (x - i >= 0 && Board[x - i, y] == obj)
                    dir[6]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (x + i < 10 && y - i >= 0 && Board[x + i, y - i] == obj)
                    dir[1]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (x + i < 10 && y + i < 10 && Board[x + i, y + i] == obj)
                    dir[3]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (x - i >= 0 && y + i < 10 && Board[x - i, y + i] == obj)
                    dir[5]++;
                else break;
            }
            for (int i = 1; i <= 4; i++)
            {
                if (x - i >= 0 && y - i >= 0 && Board[x - i, y - i] == obj)
                    dir[7]++;
                else break;
            }

            dir[0] += dir[4];
            dir[1] += dir[5];
            dir[2] += dir[6];
            dir[3] += dir[7];

            for (int i = 0; i < 4; i++)
            {
                if (dir[i] == 4)
                {
                    MessageBox.Show((who ? "Black" : "White") + " side wins!", "Winner", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    if (Internet)
                    {
                        Internet = !Internet;
                        tcp.Close();
                        Stream.Close();
                        SW.Close();
                        SR.Close();
                        backgroundWorker1.Dispose();
                    }
                    this.InitBoard();
                    return true;
                }
                
            }
            if (toolStripProgressBar1.Value == 0)
            {
                MessageBox.Show("Draw", "Message", MessageBoxButtons.OK, MessageBoxIcon.Information, MessageBoxDefaultButton.Button1);
                return true;
            }
            return false;
        }

        private void InitBoard()
        {
            this.turn = true;
            toolStripProgressBar1.Value = 0;
            toolStripProgressBar1.Value = 100;
            label1.Text = "Turn to: " + (turn ? "Black" : "White") + " Side";
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    chess[i, j].Image = null;
                    Board[i, j] = 0;
                }
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult ans = MessageBox.Show("Are you sure?", "Message", MessageBoxButtons.YesNo, 
                    MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);
            if (ans == DialogResult.Yes)
                Application.Exit();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            InitBoard();
            richTextBox1.Text = "";            
        }

        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            while(true)
            {
                try
                {
                    string s = SR.ReadLine();
                    int x, y;
                    x = int.Parse(s.Substring(0, 1));
                    y = int.Parse(s.Substring(1, 1));
                    Board[y, x] = turn ? 1 : 2;
                    this.pictureBox1.Invoke(new MethodInvoker(delegate () { chess[x, y].Image = turn ? black : white; }));
                    this.richTextBox1.Invoke(new MethodInvoker(delegate ()
                    {
                        richTextBox1.AppendText((turn ? "Black" : "White") + " side place chess piece at (" + (char)(x + 'A') + ", " + (char)(y + '0') + ")\n");
                        toolStripProgressBar1.Value--;
                    }));
                    if (referee(x, y, turn))
                    {
                        this.pictureBox1.Invoke(new MethodInvoker(delegate ()
                        {
                            toolStripStatusLabel1.Text = "Disconnected";
                            button1.Enabled = true;
                        }));
                        return;
                    }
                    turn = !turn;
                    this.label1.Invoke(new MethodInvoker(delegate () { label1.Text = "Turn to: " + (turn ? "Black" : "White") + " Side"; }));
                }
                catch { }
            }
        }

        private void statusStrip1_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {

        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void connectionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                tcp = new TcpClient("192.168.1.108", 6666);
                Stream = new NetworkStream(tcp.Client);
                SR = new StreamReader(Stream);
                SW = new StreamWriter(Stream);
                richTextBox1.AppendText("Connected!\n");
                string s = SR.ReadLine();
                if (s[0] == 0x00)
                    You = true;
                else You = false;
                InitBoard();
                Internet = true;
                turn = true;
                button1.Enabled = false;
                toolStripStatusLabel1.Text = "Connected";

                backgroundWorker1.RunWorkerAsync();
            }
            catch
            {
                MessageBox.Show("Try again later!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
