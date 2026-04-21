using System.Diagnostics;

namespace Dialog_Astrakhankina
{
    public partial class Form1 : Form
    {
        private Process _consoleProcess;
        private EventWaitHandle _hStart;
        private EventWaitHandle _hStop;
        private EventWaitHandle _hConfirm;
        private EventWaitHandle _hClose;

        private int _nextThreadNumber = 1; 

        public Form1()
        {
            InitializeComponent();
            SetupEvents();
            this.FormClosing += Form1_FormClosing;
        }

        private void SetupEvents()
        {
            _hStart = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent_Astrakhankina");
            _hStop = new EventWaitHandle(false, EventResetMode.AutoReset, "StopEvent_Astrakhankina");
            _hConfirm = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent_Astrakhankina");
            _hClose = new EventWaitHandle(false, EventResetMode.AutoReset, "CloseEvent_Astrakhankina");
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (_consoleProcess == null || _consoleProcess.HasExited)
            {
                StartConsole();
            }
            else
            {
                CreateThreads();
            }
        }

        private void StartConsole()
        {
            _consoleProcess = new Process();
            _consoleProcess.StartInfo.FileName = "Console_Astrakhankina.exe";
            _consoleProcess.StartInfo.UseShellExecute = false;
            _consoleProcess.EnableRaisingEvents = true;
            _consoleProcess.Exited += ConsoleProcess_Exited;

            _consoleProcess.Start();

            _nextThreadNumber = 1; 
            cbThreadList.Items.Clear();
            cbThreadList.Items.Add("Все потоки");
            cbThreadList.Items.Add("Главный поток");
            cbThreadList.SelectedIndex = 0;

            lblStatus.Text = "Статус: Консоль запущена";
            lblStatus.ForeColor = Color.Green;
            btnStop.Enabled = true;
        }
        private void CreateThreads()
        {
            int n = (int)numThreadCount.Value;
            for (int i = 0; i < n; i++)
            {
                _hStart.Set();
                if (_hConfirm.WaitOne(2000))
                {
                    cbThreadList.Items.Add($"Поток {_nextThreadNumber}");
                    _nextThreadNumber++;
                    Thread.Sleep(10);
                }
            }
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            if (_consoleProcess == null || _consoleProcess.HasExited) return;

            _hStop.Set();
            if (_hConfirm.WaitOne(2000))
            {
                if (cbThreadList.Items.Count > 2)
                {
                    cbThreadList.Items.RemoveAt(cbThreadList.Items.Count - 1);
                }
                else
                {
                    _consoleProcess = null;
                    ResetUI();
                }
            }
        }

        private void ConsoleProcess_Exited(object sender, EventArgs e)
        {
            if (this.IsHandleCreated)
            {
                this.Invoke(new Action(ResetUI));
            }
        }

        private void ResetUI()
        {
            lblStatus.Text = "Статус: Консоль закрыта";
            lblStatus.ForeColor = Color.Red;
            btnStop.Enabled = false;
            cbThreadList.Items.Clear();
            _nextThreadNumber = 1;  
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (_consoleProcess != null && !_consoleProcess.HasExited)
            {
                _hClose.Set();
                _consoleProcess.WaitForExit(1000);
                _consoleProcess.Kill();
            }

            _hStart?.Dispose();
            _hStop?.Dispose();
            _hConfirm?.Dispose();
            _hClose?.Dispose();
        }
    }
}