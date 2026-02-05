using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net.Http;
using System.Threading.Tasks;
using QEngine;
using QEngine.GUI;
using QEngine.Text;

public class MainScene : QEScene
{
    static readonly string Version = "0.2.2";
    static int os = OperatingSystem.IsLinux() ? 1 : OperatingSystem.IsWindows() ? 2 : 0;
    
    public override void Init()
    {
        if (os == 0) 
            return;

        Game.title = $"QEngine {Version} - Installer";
        Game.SetResolution(800, 340);
        Game.background = new(20);

        var txt = new GameObject().AddComponent<Text>();
        txt.text = $"QEngine Installer\n{Version}";
        txt.fontSize = 12;
        txt.transform.position = new(-340, 40);
        
        var back = new GameObject().AddComponent<Image>();
        back.size = new(700, 200);
        back.color = new(30);
        var progress = new GameObject().AddComponent<Slider>();
        progress.size = new(680, 60);
        progress.transform.position = new(0, -60);
        progress.hideHandle = true; progress.isInteractable = false;
        progress.fillColor = new(50, 200, 50);
        progress.SetValue(0);
        progress.SetMax(8);
        
        string zip = "Libs";

        string engineDir = "";
        if (os == 1) { var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile); engineDir = Path.Combine(home, ".local", "share", "qengine"); } // Linux
        else if (os == 2) { var home = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData); engineDir = Path.Combine(home, "qengine"); } // Windows
        
        Console.WriteLine("Creating catalog 'qengine'...");
        progress.SetValue(1);
        Directory.CreateDirectory(engineDir);
        
        Console.WriteLine("Downloading 'QEngine.dll'...");
        progress.SetValue(2);
        Download($"https://github.com/Qunerum/QEngine/releases/download/{Version}/QEngine.dll", Path.Combine(engineDir, "QEngine.dll")).Wait();
        
        Console.WriteLine($"Downloading '{zip}.zip'...");
        progress.SetValue(3);
        Download($"https://github.com/Qunerum/QEngine/releases/download/{Version}/{zip}.zip", Path.Combine(engineDir, $"{zip}.zip")).Wait();

        var libsDir = Path.Combine(engineDir, zip);
        if (Directory.Exists(libsDir))
            Directory.Delete(libsDir, true);
        
        Console.WriteLine($"Unpacking '{zip}.zip'...");
        progress.SetValue(4);
        ZipFile.ExtractToDirectory(Path.Combine(engineDir, $"{zip}.zip"), libsDir);
        
        Console.WriteLine($"Deleting '{zip}.zip'...");
        progress.SetValue(5);
        File.Delete(Path.Combine(engineDir, $"{zip}.zip"));

        Console.WriteLine("Creating runtime config...");
        progress.SetValue(6);
        CreateRuntimeConfig(engineDir);
        
        Console.WriteLine("Creating cmd file...");
        progress.SetValue(7);
        CreateCommand(engineDir);
        
        Game.title = $"QEngine {Version} - Installer - INSTALLED!";
        Console.WriteLine("Installation complete!");
        progress.SetValue(8);
    }
    
    // ---------- helpers ----------
    static async Task Download(string url, string path)
    {
        using var http = new HttpClient();
        var data = await http.GetByteArrayAsync(url);
        await File.WriteAllBytesAsync(path, data);
    }

    static void CreateRuntimeConfig(string dir)
    {
        File.WriteAllText(Path.Combine(dir, "QEngine.runtimeconfig.json"),
@"{
  ""runtimeOptions"": {
    ""tfm"": ""net8.0"",
    ""framework"": {
      ""name"": ""Microsoft.NETCore.App"",
      ""version"": ""8.0.0""
    }
  }
}");
    }

    static void CreateCommand(string dir)
    {
        if (os == 1)
        {
            var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            Directory.CreateDirectory(Path.Combine(home, ".local", "bin"));

            string script =
                $@"#!/bin/bash
            dotnet ""{dir}/QEngine.dll"" ""$@""";

            File.WriteAllText(Path.Combine(home, ".local", "bin", "qe"), script);
            Run($"chmod +x \"{Path.Combine(home, ".local", "bin", "qe")}\"");
            Run("grep -qxF 'export PATH=\"$HOME/.local/bin:$PATH\"' ~/.bashrc || " +
                "echo 'export PATH=\"$HOME/.local/bin:$PATH\"' >> ~/.bashrc");

        }
        else if (os == 2) // Windows
        {
            File.WriteAllText(Path.Combine(dir, "qe.cmd"),
                @"@echo off
dotnet ""%~dp0QEngine.dll"" %*");
        }
    }
    
    static void Run(string cmd)
    {
        if (!OperatingSystem.IsLinux()) return;

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
}