using QEngine;
using QEngine.GUI;

public class MainScene : QEScene
{
    public override void Init()
    {
        Game.title = $"QEngine ";
        Game.SetResolution(800, 600);

        if (os == 0) 
            return;

        string engineDir = "";
        if (os == 1) // Linux
        {
            Run("dotnet new install Avalonia.Templates");
            var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            engineDir = Path.Combine(home, ".local", "share", "qengine");
        } 
        else if (os == 2) // Windows
        {
            var home = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            engineDir = Path.Combine(home, "qengine");
        } 
        Print();
        Directory.CreateDirectory(engineDir);
        coms.Add("&7Creating 'qengine' directory...");
        Print();
        Download($"https://github.com/Qunerum/QEngine/releases/download/{Version}/QEngine.dll", Path.Combine(engineDir, "QEngine.dll")).Wait();
        Print();
        Download($"https://github.com/Qunerum/QEngine/releases/download/{Version}/Libs.zip", Path.Combine(engineDir, "Libs.zip")).Wait();
        Print();
        coms.Add("&7Extracting Libs...");
        var libsDir = Path.Combine(engineDir, "Libs");
        if (Directory.Exists(libsDir))
            Directory.Delete(libsDir, true);
        ZipFile.ExtractToDirectory(Path.Combine(engineDir, "Libs.zip"), libsDir);
        Print();

        File.Delete(Path.Combine(engineDir, "Libs.zip"));

        CreateRuntimeConfig(engineDir);
        CreateCommand(engineDir);
    }
    
    // ---------- helpers ----------
    static async Task Download(string url, string path)
    {
        coms.Add("&7Downloading " + Path.GetFileName(path));
        using var http = new HttpClient();
        var data = await http.GetByteArrayAsync(url);
        Print();
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
