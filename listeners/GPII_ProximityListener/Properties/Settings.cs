// Decompiled with JetBrains decompiler
// Type: GPII.Properties.Settings
// Assembly: GPIIWindows8, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: EEE1BFC0-0D05-42C0-BFC7-1DFF997E6E01
// Assembly location: S:\gpii\other\gpii-proximity-listener\orig\GPIIWindowsProximityListener.exe.exe

using System.CodeDom.Compiler;
using System.Configuration;
using System.Runtime.CompilerServices;

namespace GPII.Properties
{
  [CompilerGenerated]
  [GeneratedCode("Microsoft.VisualStudio.Editors.SettingsDesigner.SettingsSingleFileGenerator", "12.0.0.0")]
  internal sealed class Settings : ApplicationSettingsBase
  {
    private static Settings defaultInstance = (Settings) SettingsBase.Synchronized((SettingsBase) new Settings());

    public static Settings Default
    {
      get
      {
        Settings defaultInstance = Settings.defaultInstance;
        return defaultInstance;
      }
    }
  }
}
