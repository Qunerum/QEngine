using System.Runtime.InteropServices;
using System.Text.Json;
using QEConsole;

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

                FileManager.CopyDirectory(Path.Combine(engineDir, "ProjectTemplate"), name);
                
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
        if (args.Length != 1 || !new[] { "win", "windows", "linux" }.Contains(args[0].ToLower()))
        {
            Writer.Write($"&4Usage: qe build <platform>");
            Writer.Write($"&4Available platforms: Windows/Win , Linux");
            return;
        }
        
        Writer.Write($"&7Building project at '{projectRoot}'...");

        if (Directory.Exists(Path.Combine(projectRoot, "Build"))) { FileManager.ClearDirectory(Path.Combine(projectRoot, "Build")); }

        if (!Directory.Exists(Path.Combine(projectRoot, "Build")))
        {
            Writer.Write($"&7Creating folder 'Build' at '{projectRoot}'...");
            Directory.CreateDirectory(Path.Combine(projectRoot, "Build"));
        }

        Console.ForegroundColor = ConsoleColor.DarkGray;

        string plat = (args[0].ToLower() == "windows" || args[0].ToLower() == "win") ? "win-x64" : "linux-x64";
        Terminal.RunCommand("dotnet", $"publish -c Release -r {plat}");
        //Copy assets
        Writer.Write($"&7Reading project data from '.qeproject' at '{projectRoot}'...");
        using JsonDocument doc = JsonDocument.Parse(File.ReadAllText(Path.Combine(projectRoot, ".qeproject")));
        JsonElement root = doc.RootElement;
        string projectName = root.GetProperty("name").GetString()!;
        Writer.Write($"&7Project name is '{projectName}'");

        Writer.Write($"&7Moving '{projectName}' to 'Build'...");
        string publishDir = Path.Combine(projectRoot, "bin", "Release", "net8.0", plat, "publish");
        var exeFile = Directory.GetFiles(publishDir)
            .FirstOrDefault(f => !f.EndsWith(".dll") && !f.EndsWith(".pdb") && !f.EndsWith(".json"));
        if (exeFile == null)
            throw new FileNotFoundException("Cannot find published executable in " + publishDir);
        File.Move(exeFile, Path.Combine(projectRoot, "Build", Path.GetFileName(exeFile)), true);
        Writer.Write($"&2'{projectName}' moved to 'Build' successfully!");
        
        Writer.Write($"&2Copying 'Assets' to 'Build/Assets'...");
        FileManager.CopyDirectory(Path.Combine(projectRoot, "Assets"), Path.Combine(projectRoot, "Build", "Assets"));
        FileManager.CopyDirectory(Path.Combine(projectRoot, "Fonts"), Path.Combine(projectRoot, "Build", "Fonts"));

        Writer.Write("&2Build finished!");
    }
}

class RunCommand : ICommand
{
    public string Name => "run";
    public string Description => "Run project: qe run";
    public bool RequiresProject => true;

    public void Execute(string[] args, string? projectRoot)
    {
        Writer.Write($"&7Running project at '{projectRoot}'...");
        Writer.Write($"&7Reading project data from '.qeproject' at '{projectRoot}'...");
        using JsonDocument doc = JsonDocument.Parse(File.ReadAllText(Path.Combine(projectRoot, ".qeproject")));
        JsonElement root = doc.RootElement;
        string projectName = root.GetProperty("name").GetString()!;
        Writer.Write($"&7Project name is '{projectName}'");
        Writer.Write($"&7Running 'Build/{projectName}{(OperatingSystem.IsWindows() ? ".exe" : "")}'...");
        Terminal.RunCommand($"Build/{projectName}{(OperatingSystem.IsWindows() ? ".exe" : "")}");
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

class UpdateCommand : ICommand
{
    public string Name => "update";
    public string Description => "Update project: qe update";
    public bool RequiresProject => true;
    public void Execute(string[] args, string? projectRoot)
    {
        var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
        string engineDir = "";
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        { engineDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "qengine"); } else
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        { engineDir = Path.Combine(home, ".local", "share", "qengine"); }

        Writer.Write($"&7Copying 'Assets.cs'...");
        File.WriteAllText(Path.Combine(projectRoot, ".EngineScripts", "Assets.cs"),
            File.ReadAllText(Path.Combine(engineDir, "Libs", "Assets.cs")));
        Writer.Write($"&7Copying 'Core.cs'...");
        File.WriteAllText(Path.Combine(projectRoot, ".EngineScripts", "Core.cs"),
            File.ReadAllText(Path.Combine(engineDir, "Libs", "Core.cs")));
        Writer.Write($"&7Copying 'QEngine.cs'...");
        File.WriteAllText(Path.Combine(projectRoot, ".EngineScripts", "QEngine.cs"),
            File.ReadAllText(Path.Combine(engineDir, "Libs", "QEngine.cs")));
        Writer.Write($"&7Copying 'Renderer.cs'...");
        File.WriteAllText(Path.Combine(projectRoot, ".EngineScripts", "Renderer.cs"),
            File.ReadAllText(Path.Combine(engineDir, "Libs", "Renderer.cs")));
    }
}