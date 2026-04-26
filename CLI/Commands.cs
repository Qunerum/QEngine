using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Text.Json;
using System.IO;
using QEConsole;
using System;

class NewProjectCommand : ICommand
{
    public string Name => "new";
    public string Description => "Create new project / scene / script: qe create new ...";
    public bool RequiresProject => false;

    public void Execute(string[] args, string? _)
    {
        string type = args[0].Trim().ToLower();
        if (args.Length != 2 || !new[] { "project", "scene", "script", "component" }.Contains(type))
        {
            Writer.Write("&4Usage: &6qe new project <proj name>");
            Writer.Write("&4Usage: &6qe new scene <scene name>");
            Writer.Write("&4Usage: &6qe new script <script name>");
            return;
        }
        string name = args[1].Replace('-', '_');

        switch (type)
        {
            case "project":
                var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                string engineDir = "";

                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) { engineDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "qengine"); } else
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) { engineDir = Path.Combine(home, ".local", "share", "qengine"); }

                FileManager.CopyDirectory(Path.Combine(engineDir, "TemplateProject"), name);
                
                File.Copy(Path.Combine(name, "TemplateProject.csproj"), Path.Combine(name, $"{name}.csproj"), true);
                File.Delete(Path.Combine(name, "TemplateProject.csproj"));
                File.Delete(Path.Combine(name, ".qeproject"));
                File.WriteAllText(Path.Combine(name, ".qeproject"), $"{{\n    \"name\": \"{name}\",\n    \"engineVersion\": \"{QEngineData.version}\"\n}}");

                Writer.Write($"&2Project '{name}' created!");
                break;
            case "scene":
                if (File.Exists(Path.Combine(_, ".qeproject")))
                {
                    Writer.Write($"&7Creating '{name}.cs'...");
                    File.WriteAllText(Path.Combine(_, "Scripts", $"{name}.cs"),
                        "using QEngine;\n" +
                        $"public class {name} : QEScene\n" +
                        "{\n" +
                        "    public override void Init()\n" +
                        "    {\n" +
                        "        //Your code here\n" +
                        "    }\n" +
                        "}");
                }
                break;
            case "script":
                if (File.Exists(Path.Combine(_, ".qeproject")))
                {
                    Writer.Write($"&7Creating '{name}.cs'...");
                    File.WriteAllText(Path.Combine(_, "Scripts", $"{name}.cs"),
                        "using QEngine;\n" +
                        $"public class {name} : QEScript\n" +
                        "{\n" +
                        "    public override void Init()\n" +
                        "    {\n" +
                        "        //Your code here\n" +
                        "    }\n" +
                        "    public override void Update()\n" +
                        "    {\n" +
                        "        //Your code here\n" +
                        "    }\n" +
                        "}");
                }
                break;
        }
    }
}

class BuildCommand : ICommand
{
    public string Name => "build";
    public string Description => "Build current project: qe build";
    public bool RequiresProject => true;

    public void Execute(string[] args, string? projectRoot)
    {
        if (args.Length != 1 || !new[] { "win", "windows", "linux", "both" }.Contains(args[0].ToLower()))
        {
            Writer.Write($"&4Usage: qe build <platform>");
            Writer.Write($"&4Available platforms: windows / win , linux , both");
            Writer.Write($"&4> windows / win");
            Writer.Write($"&4> linux");
            Writer.Write($"&4> both");
            return;
        }
        
        Writer.Write($"&7Building project at '{projectRoot}'...");

        if (Directory.Exists(Path.Combine(projectRoot, "Build"))) { FileManager.ClearDirectory(Path.Combine(projectRoot, "Build")); }

        Console.ForegroundColor = ConsoleColor.DarkGray;

        if (args[0].ToLower() != "both")
        {
            string plat = (args[0].ToLower() == "windows" || args[0].ToLower() == "win") ? "win-x64" : "linux-x64";
            if (!Directory.Exists(Path.Combine(projectRoot, "Build", plat)))
            {
                Writer.Write($"&7Creating folder 'Build/{plat}' at '{projectRoot}'...");
                Directory.CreateDirectory(Path.Combine(projectRoot, "Build", plat));
            }
            Build(plat, args, projectRoot);
        }
        else
        {
            if (!Directory.Exists(Path.Combine(projectRoot, "Build", "linux-x64")) || !Directory.Exists(Path.Combine(projectRoot, "Build", "win-x64")))
            {
                Writer.Write($"&7Creating folder 'Build/linux-x64' at '{projectRoot}'...");
                Writer.Write($"&7Creating folder 'Build/win-x64' at '{projectRoot}'...");
                Directory.CreateDirectory(Path.Combine(projectRoot, "Build", "linux-x64"));
                Directory.CreateDirectory(Path.Combine(projectRoot, "Build", "win-x64"));
            }
            Build("win-x64", args, projectRoot);
            Build("linux-x64", args, projectRoot);
        }

        Writer.Write("&2Build finished!");
    }

    void Build(string plat, string[] args, string? projectRoot)
    {
        Terminal.RunCommand("dotnet", $"publish -c Release -r {plat}");
        //Copy assets
        Writer.Write($"&7Reading project data from '.qeproject' at '{projectRoot}'...");
        using JsonDocument doc = JsonDocument.Parse(File.ReadAllText(Path.Combine(projectRoot, ".qeproject")));
        JsonElement root = doc.RootElement;
        string projectName = root.GetProperty("name").GetString()!;
        Writer.Write($"&7Project name is '{projectName}'");

        Writer.Write($"&7Moving '{projectName}' to 'Build/{plat}'...");
        string publishDir = Path.Combine(projectRoot, "bin", "Release", "net8.0", plat, "publish");
        var exeFile = Directory.GetFiles(publishDir)
            .FirstOrDefault(f => !f.EndsWith(".dll") && !f.EndsWith(".pdb") && !f.EndsWith(".json"));
        if (exeFile == null)
            throw new FileNotFoundException("Cannot find published executable in " + publishDir);
        File.Move(exeFile, Path.Combine(projectRoot, "Build", plat, Path.GetFileName(exeFile)), true);
        Writer.Write($"&2'{projectName}' moved to 'Build/{plat}' successfully!");

        Writer.Write($"&2Copying 'Assets' to 'Build/{plat}/Assets'...");
        FileManager.CopyDirectory(Path.Combine(projectRoot, "Assets"),
            Path.Combine(projectRoot, "Build", plat, "Assets"));
        FileManager.CopyDirectory(Path.Combine(projectRoot, "Fonts"), 
            Path.Combine(projectRoot, "Build", plat, "Fonts"));
    }
}


class RunCommand : ICommand
{
    public string Name => "run";
    public string Description => "Run project: qe run";
    public bool RequiresProject => true;

    public void Execute(string[] args, string? projectRoot)
    {
        if (string.IsNullOrEmpty(projectRoot)) return;

        Writer.Write($"&7Running project at '{projectRoot}'...");
        
        string configPath = Path.Combine(projectRoot, ".qeproject");
        if (!File.Exists(configPath))
        {
            Writer.Write("&cError: .qeproject file not found!");
            return;
        }

        using JsonDocument doc = JsonDocument.Parse(File.ReadAllText(configPath));
        JsonElement root = doc.RootElement;
        string projectName = root.GetProperty("name").GetString()!;
        
        string platformDir = OperatingSystem.IsWindows() ? "win-x64" : "linux-x64";
        string extension = OperatingSystem.IsWindows() ? ".exe" : "";
        string binaryName = projectName + extension;
        
        string fullBuildPath = Path.GetFullPath(Path.Combine(projectRoot, "Build", platformDir, binaryName));

        if (!File.Exists(fullBuildPath))
        {
            Writer.Write($"&cError: Binary not found at '{fullBuildPath}'. Did you build the project?");
            return;
        }

        Writer.Write($"&7Starting '{binaryName}' in a new window...");

        try
        {
            string workingDir = Path.GetDirectoryName(fullBuildPath) ?? projectRoot;

            if (OperatingSystem.IsWindows())
            {
                // Na Windows: 'start' potrzebuje pustego tytułu w cudzysłowie, 
                // aby poprawnie zinterpretować ścieżkę do pliku jako komendę.
                Process.Start(new ProcessStartInfo
                {
                    FileName = "cmd.exe",
                    Arguments = $"/c start \"{projectName}\" cmd /k \"\"{fullBuildPath}\"\"",
                    WorkingDirectory = workingDir,
                    UseShellExecute = true
                });
            }
            else if (OperatingSystem.IsLinux())
            {
                Process.Start("chmod", $"+x \"{fullBuildPath}\"").WaitForExit();

                // Linux: Większość terminali potrzebuje flagi -e lub -x, 
                // ale najbezpieczniej jest wywołać powłokę (sh), która odpali apkę.
                string[] terminalEmulators = { "x-terminal-emulator", "gnome-terminal", "konsole", "xfce4-terminal", "xterm" };
                bool started = false;

                foreach (var term in terminalEmulators)
                {
                    try
                    {
                        // Używamy "sh -c", aby terminal wiedział, że ma wykonać plik binarny i nie zamknąć się od razu
                        string linuxArgs = term switch
                        {
                            "gnome-terminal" => $"-- sh -c \"'{fullBuildPath}'; exec sh\"",
                            "konsole" => $"-e sh -c \"'{fullBuildPath}'; exec sh\"",
                            _ => $"-e \"{fullBuildPath}\""
                        };

                        Process.Start(new ProcessStartInfo
                        {
                            FileName = term,
                            Arguments = linuxArgs,
                            WorkingDirectory = workingDir
                        });
                        started = true;
                        break;
                    }
                    catch { }
                }

                if (!started)
                {
                    Process.Start(new ProcessStartInfo { FileName = fullBuildPath, WorkingDirectory = workingDir });
                }
            }
        } catch { }
    }
}
class BARCommand : ICommand
{
    public string Name => "buildrun";
    public string Description => "Build and run project: qe buildrun or qe bar";
    public bool RequiresProject => true;

    public void Execute(string[] args, string? projectRoot)
    {
        if (args.Length == 0)
        {
            Terminal.RunCommand("qe", $"build {(OperatingSystem.IsWindows() ? "win" : "linux")}");
            Terminal.RunCommand("qe", "run");
        } else 
        {
            Writer.Write($"&4Usage: qe buildrun"); 
            Writer.Write($"&4Usage: qe bar"); 
        }
    }
}

class SearchCommand : ICommand
{
    public string Name => "search";
    public string Description => "Search projects: qe search";
    public bool RequiresProject => false;

    public void Execute(string[] args, string? projectRoot)
    {
        var roots = FileManager.GetSearchRoots();

        var results = new List<string>();

        foreach (var root in roots)
        {
            if (!Directory.Exists(root))
                continue;
            try
            { int i = 0; foreach (var file in Directory.EnumerateFiles(root, "*.qeproject", SearchOption.AllDirectories)) 
                { i++; results.Add($"{i}. {Path.GetDirectoryName(file)!}"); } } catch { } 
        }
        if (results.Count == 0) { Writer.Write("&4No QEngine projects found."); return; }
        Writer.Write("&6Found projects:");
        foreach (var p in results) { Writer.Write("&7 - &e" + p); }
    }
}