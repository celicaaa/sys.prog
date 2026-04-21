using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace Dialog_Astrakhankina
{
    // MMF Header structure matching C++ struct
    [StructLayout(LayoutKind.Sequential)]
    struct MMFHeader
    {
        public int addr;      // Target address (-1 = all threads, 0 = main thread, >0 = thread ID)
        public int cmdType;   // Command type (0 = close, 1 = data)
        public int size;      // Data size in bytes
    }

    // DllImport for transport functions
    class TransportDll
    {
        [DllImport("TransportDLL_Astrakhankina.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr CreateTransport();

        [DllImport("TransportDLL_Astrakhankina.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DestroyTransport(IntPtr transport);

        [DllImport("TransportDLL_Astrakhankina.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool TransportSend(IntPtr transport, int addr, int cmdType, string data, int dataSize);

        [DllImport("TransportDLL_Astrakhankina.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool TransportReceive(IntPtr transport, ref MMFHeader header, StringBuilder buffer, int bufferSize, ref int outSize);
    }

    public partial class Form1 : Form
    {
        private Process _consoleProcess;
        private EventWaitHandle _hStart;
        private EventWaitHandle _hStop;
        private EventWaitHandle _hConfirm;
        private EventWaitHandle _hClose;
        private EventWaitHandle _hMessage;  // Event for signaling message ready

        private int _nextThreadNumber = 1; 
        
        // Transport instance
        private IntPtr _transport = IntPtr.Zero;

        public Form1()
        {
            InitializeComponent();
            SetupEvents();
            this.FormClosing += Form1_FormClosing;
            
            // Initialize transport
            _transport = TransportDll.CreateTransport();
        }

        private void SetupEvents()
        {
            _hStart = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent_Astrakhankina");
            _hStop = new EventWaitHandle(false, EventResetMode.AutoReset, "StopEvent_Astrakhankina");
            _hConfirm = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent_Astrakhankina");
            _hClose = new EventWaitHandle(false, EventResetMode.AutoReset, "CloseEvent_Astrakhankina");
            _hMessage = new EventWaitHandle(false, EventResetMode.AutoReset, "MessageEvent_Astrakhankina");
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
            
            // Add Send button handler
            Button btnSend = this.Controls.Find("btnSend", true).FirstOrDefault() as Button;
            if (btnSend != null)
            {
                btnSend.Click += BtnSend_Click;
                btnSend.Enabled = true;
            }
        }
        
        private void BtnSend_Click(object sender, EventArgs e)
        {
            SendMessageToConsole();
        }
        
        private void SendMessageToConsole()
        {
            if (_transport == IntPtr.Zero) return;
            
            // Get text from textbox (need to add txtMessage to form)
            TextBox txtMessage = this.Controls.Find("txtMessage", true).FirstOrDefault() as TextBox;
            if (txtMessage == null || string.IsNullOrEmpty(txtMessage.Text))
            {
                MessageBox.Show("Введите текст для отправки");
                return;
            }
            
            string messageText = txtMessage.Text;
            
            // Get selected target from combo box
            string selected = cbThreadList.SelectedItem?.ToString() ?? "Все потоки";
            int targetAddr = -1; // Default: all threads
            
            if (selected == "Главный поток")
            {
                targetAddr = 0;
            }
            else if (selected.StartsWith("Поток "))
            {
                // Parse thread number
                if (int.TryParse(selected.Substring(6), out int threadNum))
                {
                    targetAddr = threadNum;
                }
            }
            // else "Все потоки" -> addr = -1
            
            // Send via MMF transport
            int dataSize = messageText.Length * 2; // wchar_t size
            bool result = TransportDll.TransportSend(_transport, targetAddr, 1, messageText, dataSize);
            
            if (result)
            {
                // Signal that message is ready
                _hMessage.Set();
                
                // Wait for confirmation
                if (_hConfirm.WaitOne(2000))
                {
                    MessageBox.Show($"Сообщение отправлено: {selected}\nПодтверждение получено.");
                }
                else
                {
                    MessageBox.Show("Сообщение отправлено, но подтверждение не получено.");
                }
            }
            else
            {
                MessageBox.Show("Ошибка при отправке сообщения.");
            }
            
            txtMessage.Clear();
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
            _hMessage?.Dispose();
            
            // Cleanup transport
            if (_transport != IntPtr.Zero)
            {
                TransportDll.DestroyTransport(_transport);
                _transport = IntPtr.Zero;
            }
        }
    }
}