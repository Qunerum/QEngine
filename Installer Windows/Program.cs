using System.IO.Compression;
using Microsoft.Win32;

public class QEInstallerWindows
{
    static readonly string Version = "0.2.1";

    static void Main()
    {
        Console.WriteLine("=== QEngine Installer (Windows) ===");

        var baseDir = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        var engineDir = Path.Combine(baseDir, "qengine");

        Directory.CreateDirectory(engineDir);

        Download(
            $"https://github.com/Qunerum/QEngine/releases/download/{Version}/QEngine.dll",
            Path.Combine(engineDir, "QEngine.dll")
        ).Wait();

        Download(
            $"https://github.com/Qunerum/QEngine/releases/download/{Version}/Libs.zip",
            Path.Combine(engineDir, "Libs.zip")
        ).Wait();

        Console.WriteLine("Extracting Libs...");
        var libsDir = Path.Combine(engineDir, "Libs");
        if (Directory.Exists(libsDir))
            Directory.Delete(libsDir, true);

        ZipFile.ExtractToDirectory(
            Path.Combine(engineDir, "Libs.zip"),
            libsDir
        );

        File.Delete(Path.Combine(engineDir, "Libs.zip"));

        CreateRuntimeConfig(engineDir);
        CreateCommand(engineDir);
        AddToPath(engineDir);

        Console.WriteLine("QEngine installed successfully!");
        Console.WriteLine("Restart terminal to use `qe`.");
    }

    // ---------- helpers ----------

    static async Task Download(string url, string path)
    {
        Console.WriteLine("Downloading " + Path.GetFileName(path));
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
        File.WriteAllText(Path.Combine(dir, "qe.cmd"),
@"@echo off
dotnet ""%~dp0QEngine.dll"" %*");
    }

    static void AddToPath(string dir)
    {
        var key = Registry.CurrentUser.OpenSubKey("Environment", true)!;
        var path = key.GetValue("PATH")?.ToString() ?? "";

        if (!path.Contains(dir, StringComparison.OrdinalIgnoreCase))
        {
            key.SetValue("PATH", path + ";" + dir);
            Console.WriteLine("Added to PATH");
        }
    }
}
