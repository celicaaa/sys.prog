namespace Dialog_Astrakhankina
{
    partial class Form1
    {
        private System.ComponentModel.IContainer components = null;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            numThreadCount = new NumericUpDown();
            cbThreadList = new ComboBox();
            lblStatus = new Label();
            btnStart = new Button();
            btnStop = new Button();
            lblCount = new Label();
            lblList = new Label();
            ((System.ComponentModel.ISupportInitialize)numThreadCount).BeginInit();
            SuspendLayout();
 
            numThreadCount.Location = new Point(171, 29);
            numThreadCount.Margin = new Padding(3, 4, 3, 4);
            numThreadCount.Maximum = new decimal(new int[] { 10, 0, 0, 0 });
            numThreadCount.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            numThreadCount.Name = "numThreadCount";
            numThreadCount.Size = new Size(80, 27);
            numThreadCount.TabIndex = 4;
            numThreadCount.Value = new decimal(new int[] { 1, 0, 0, 0 });

            cbThreadList.DropDownStyle = ComboBoxStyle.DropDownList;
            cbThreadList.Location = new Point(23, 180);
            cbThreadList.Margin = new Padding(3, 4, 3, 4);
            cbThreadList.Name = "cbThreadList";
            cbThreadList.Size = new Size(502, 28);
            cbThreadList.TabIndex = 2;
 
            lblStatus.AutoSize = true;
            lblStatus.ForeColor = Color.Gray;
            lblStatus.Location = new Point(23, 520);
            lblStatus.Name = "lblStatus";
            lblStatus.Size = new Size(132, 20);
            lblStatus.TabIndex = 0;
            lblStatus.Text = "Статус: Ожидание";
   
            btnStart.BackColor = Color.LightGreen;
            btnStart.FlatStyle = FlatStyle.Flat;
            btnStart.Location = new Point(23, 80);
            btnStart.Margin = new Padding(3, 4, 3, 4);
            btnStart.Name = "btnStart";
            btnStart.Size = new Size(160, 47);
            btnStart.TabIndex = 0;
            btnStart.Text = "Start";
            btnStart.UseVisualStyleBackColor = false;
            btnStart.Click += btnStart_Click;

            btnStop.BackColor = Color.LightCoral;
            btnStop.Enabled = false;
            btnStop.FlatStyle = FlatStyle.Flat;
            btnStop.Location = new Point(206, 80);
            btnStop.Margin = new Padding(3, 4, 3, 4);
            btnStop.Name = "btnStop";
            btnStop.Size = new Size(160, 47);
            btnStop.TabIndex = 1;
            btnStop.Text = "Stop";
            btnStop.UseVisualStyleBackColor = false;
            btnStop.Click += btnStop_Click;

            lblCount.AutoSize = true;
            lblCount.Location = new Point(23, 33);
            lblCount.Name = "lblCount";
            lblCount.Size = new Size(154, 20);
            lblCount.TabIndex = 5;
            lblCount.Text = "Количество потоков:";

            lblList.AutoSize = true;
            lblList.Location = new Point(23, 147);
            lblList.Name = "lblList";
            lblList.Size = new Size(133, 20);
            lblList.TabIndex = 3;
            lblList.Text = "Активные потоки:";

            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(549, 568);
            Controls.Add(lblStatus);
            Controls.Add(cbThreadList);
            Controls.Add(lblList);
            Controls.Add(btnStop);
            Controls.Add(btnStart);
            Controls.Add(numThreadCount);
            Controls.Add(lblCount);
            Margin = new Padding(3, 4, 3, 4);
            Name = "Form1";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Диалоговое приложение - Astrakhankina";
            ((System.ComponentModel.ISupportInitialize)numThreadCount).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        private System.Windows.Forms.NumericUpDown numThreadCount;
        private System.Windows.Forms.ComboBox cbThreadList;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.Button btnStop;
        private System.Windows.Forms.Label lblCount;
        private System.Windows.Forms.Label lblList;
    }
}