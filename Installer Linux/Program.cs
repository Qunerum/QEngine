using System.Diagnostics;
using System.IO.Compression;

public class QEInstaller
{
    static void Main()
    {
        string ver = "0.1.1";
        
        var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);

        var engineDir = Path.Combine(home, ".local", "share", "qengine");
        var qeJs = Path.Combine(engineDir, "QEngine.runtimeconfig.json");
        var binDir = Path.Combine(home, ".local", "bin");
        var qeCmd = Path.Combine(binDir, "qe");

        Write("§a<==== §6QEngine Installer §a====>");

        // 1. Create directories
        Run($"mkdir -p \"{engineDir}\"");
        Run($"mkdir -p \"{binDir}\"");

        // 2. Download DLL
        Write("§7Downloading QEngine.dll...");
        Run($"curl -L https://github.com/Qunerum/QEngine/releases/download/{ver}/QEngine.dll -o \"{engineDir}/QEngine.dll\"");
        Write("§7Downloading Libs.zip...");
        Run($"curl -L https://github.com/Qunerum/QEngine/releases/download/{ver}/Libs.zip -o \"{engineDir}/Libs.zip\"");
        Write("§7Extracting Libs...");
        if (Directory.Exists($"{engineDir}/Libs"))
            Directory.Delete($"{engineDir}/Libs", true);
        ZipFile.ExtractToDirectory($"{engineDir}/Libs.zip", $"{engineDir}/Libs");
        Write("&7Deleting Libs.zip...");
        File.Delete($"{engineDir}/Libs.zip");

        // 3. Create qe command
        Write("§7Creating 'qe' command...");

        string js = 
@"{
  ""runtimeOptions"": {
    ""tfm"": ""net8.0"",
    ""framework"": {
      ""name"": ""Microsoft.NETCore.App"",
      ""version"": ""8.0.0""
    },
    ""configProperties"": {
      ""System.Reflection.Metadata.MetadataUpdater.IsSupported"": false,
      ""System.Runtime.Serialization.EnableUnsafeBinaryFormatterSerialization"": false
    }
  }
}";

        File.WriteAllText(qeJs, js);
        
        string script =
            $@"#!/bin/bash
            dotnet ""{engineDir}/QEngine.dll"" ""$@""";

        File.WriteAllText(qeCmd, script);

        // 4. Make executable
        Run($"chmod +x \"{qeCmd}\"");

        Write($"§aQEngine v{ver} installed!");
        Write("§aInstallation complete!");
        
        Run("echo 'export PATH=\"$HOME/.local/bin:$PATH\"' >> ~/.bashrc");
        Run("source ~/.bashrc");
    }

    static void Run(string cmd)
    {
        var p = new Process();
        p.StartInfo.FileName = "bash";
        p.StartInfo.Arguments = "-c \"" + cmd.Replace("\"", "\\\"") + "\"";
        p.StartInfo.RedirectStandardOutput = true;
        p.StartInfo.RedirectStandardError = true;
        p.StartInfo.UseShellExecute = false;

        p.Start();
        Console.Write(p.StandardOutput.ReadToEnd());
        Console.Write(p.StandardError.ReadToEnd());
        p.WaitForExit();
    }

    static void Write(string msg)
    {
        Console.WriteLine(msg
            .Replace("§a", "\u001b[32m")
            .Replace("§6", "\u001b[33m")
            .Replace("§7", "\u001b[90m")
            .Replace("§e", "\u001b[93m")
            + "\u001b[0m");
    }
}