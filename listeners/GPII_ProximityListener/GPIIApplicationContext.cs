// Decompiled with JetBrains decompiler
// Type: GPII.GPIIApplicationContext
// Assembly: GPIIWindows8, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: EEE1BFC0-0D05-42C0-BFC7-1DFF997E6E01
// Assembly location: S:\gpii\other\gpii-proximity-listener\orig\GPIIWindowsProximityListener.exe.exe

using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace GPII
{
  public class GPIIApplicationContext : ApplicationContext
  {
    private IContainer components;
    private NotifyIcon notifyIcon;
    private GPIIProximityListener p;

    public GPIIApplicationContext()
    {
      this.components = (IContainer) new Container();
      this.notifyIcon = new NotifyIcon(this.components)
      {
        ContextMenuStrip = new ContextMenuStrip(),
        Icon = Properties.Resources.GPII,
        Text = "GPII Windows Proximity Listener (2017-08-04)",
        Visible = true
      };
      this.notifyIcon.ContextMenuStrip.Items.Add((ToolStripItem) this.ToolStripMenuItemWithHandler("&Exit", new EventHandler(this.exitItem_Click)));
      this.p = new GPIIProximityListener();
      this.p.InitializeProximityDevice();
    }

    protected override void Dispose(bool disposing)
    {
      if (!disposing || this.components == null)
        return;
      this.components.Dispose();
    }

    private void exitItem_Click(object sender, EventArgs e)
    {
      foreach (Process process in Process.GetProcessesByName("node"))
        process.Kill();
      this.ExitThread();
    }

    private void ContextMenuStrip_Opening(object sender, CancelEventArgs e)
    {
      e.Cancel = false;
      this.notifyIcon.ContextMenuStrip.Items.Add((ToolStripItem) this.ToolStripMenuItemWithHandler("&Exit", new EventHandler(this.exitItem_Click)));
    }

    private ToolStripMenuItem ToolStripMenuItemWithHandler(string displayText, int enabledCount, int disabledCount, EventHandler eventHandler)
    {
      ToolStripMenuItem toolStripMenuItem = new ToolStripMenuItem(displayText);
      if (eventHandler != null)
        toolStripMenuItem.Click += eventHandler;
      toolStripMenuItem.Image = (Image) null;
      toolStripMenuItem.ToolTipText = "";
      return toolStripMenuItem;
    }

    public ToolStripMenuItem ToolStripMenuItemWithHandler(string displayText, EventHandler eventHandler)
    {
      return this.ToolStripMenuItemWithHandler(displayText, 0, 0, eventHandler);
    }
  }
}
