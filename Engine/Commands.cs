// -------------------------
// Komendy
// -------------------------

using System.Text.Json;
using QEConsole;

class ProjectData
{
    public string name { get; set; }
    public string engineVersion { get; set; }
}

class NewProjectCommand : ICommand
{
    public string Name => "new";
    public string Description => "Create new project: qe new project <name>";
    public bool RequiresProject => false;

    public void Execute(string[] args, string? _)
    {
        if (args.Length < 2 || args[0] != "project")
        {
            Writer.Write("&4Usage: &6qe new project <name>");
            return;
        }

        string name = args[1].Replace(" ", "");
        Directory.CreateDirectory(name);

        Terminal.RunCommand("dotnet", $"new avalonia.app -n {name}"); // Create project

        File.Delete(Path.Combine(name, "App.axaml")); File.Delete(Path.Combine(name, "App.axaml.cs")); // Delete App.axaml and App.axaml.cs
        File.Delete(Path.Combine(name, "MainWindow.axaml")); File.Delete(Path.Combine(name, "MainWindow.axaml.cs")); // Delete MainWindow.axaml and MainWindow.axaml.cs
        File.Delete(Path.Combine(name, "Program.cs")); File.Delete(Path.Combine(name, "app.manifest")); // Delete Program.cs and app.manifest

        File.WriteAllText(Path.Combine(name, $"{name}.csproj"),
            "" +
            "<Project Sdk=\"Microsoft.NET.Sdk\">\n" +
            "    <PropertyGroup>\n" +
            "        <OutputType>WinExe</OutputType>\n" +
            "        <TargetFramework>net8.0</TargetFramework>\n" +
            "        <Nullable>enable</Nullable>\n" +
            "        <AvaloniaUseCompiledBindingsByDefault>true</AvaloniaUseCompiledBindingsByDefault>\n" +
            "        <DebugType>none</DebugType>\n" +
            "        <RuntimeIdentifiers>win-x64;linux-x64</RuntimeIdentifiers>\n" +
            "        <PublishSingleFile>true</PublishSingleFile>\n" +
            "        <SelfContained>true</SelfContained>\n" +
            "        <IncludeAllContentForSelfExtract>true</IncludeAllContentForSelfExtract>\n" +
            "        <PublishTrimmed>false</PublishTrimmed>\n" +
            "    </PropertyGroup>\n" +
            "\n" +
            "    <ItemGroup>\n" +
            "        <Compile Include=\".EngineScripts\\**\\*.cs\" />\n" +
            "        \n" +
            "        <PackageReference Include=\"Avalonia\" Version=\"11.3.10\"/>\n" +
            "        <PackageReference Include=\"Avalonia.Desktop\" Version=\"11.3.10\"/>\n" +
            "        <PackageReference Include=\"Avalonia.Themes.Fluent\" Version=\"11.3.10\"/>\n" +
            "        <PackageReference Include=\"Avalonia.Fonts.Inter\" Version=\"11.3.10\"/>\n" +
            "    </ItemGroup>\n" +
            "</Project>");
        Directory.CreateDirectory(Path.Combine(name, "Assets"));
        Directory.CreateDirectory(Path.Combine(name, "Scripts"));
        Directory.CreateDirectory(Path.Combine(name, ".EngineScripts"));
        
        File.WriteAllText(Path.Combine(name, ".EngineScripts", "Assets.cs"), Scripts.AssetsCs);
        File.WriteAllText(Path.Combine(name, ".EngineScripts", "Core.cs"), Scripts.CoreCs);
        File.WriteAllText(Path.Combine(name, ".EngineScripts", "QEngine.cs"), Scripts.QEngineCs);
        File.WriteAllText(Path.Combine(name, ".EngineScripts", "Renderer.cs"), Scripts.RendererCs);
        
        File.WriteAllText(Path.Combine(name, "Scripts", "MainScene.cs"),
            "using QEngine;\n" +
            "using QEngine.GUI;\n\n" +
            $"public class MainScene : QEScene\n" +
            "{\n" +
            "    public override void Init()\n" +
            "    {\n" +
            "        Game.title = \"Your Game Title\";\n" +
            "        Game.size = new(800, 600);\n" +
            "        \n" +
            "        CreateObject(\"Object\").AddComponent<Image>().color = new(100);\n" +
            "    }\n" +
            "}");

        File.WriteAllText(Path.Combine(name, QEngineData.projFile),
            "{\n" +
            $"    \"name\": \"{name}\",\n" +
            $"    \"engineVersion\": \"{QEngineData.version}\"\n" +
            "}"); // Project file

        Writer.Write($"&2Project '{name}' created!");
    }
}

class BuildCommand : ICommand
{
    public string Name => "build";
    public string Description => "Build current project";
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

        if (Directory.Exists(Path.Combine(projectRoot, "Build"))) { Directory.Delete(Path.Combine(projectRoot, "Build"), true); }

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

        Writer.Write("&2Build finished!");
    }
}

class OpenCommand : ICommand
{
    public string Name => "open";
    public string Description => "Open project";
    public bool RequiresProject => true;

    public void Execute(string[] args, string? projectRoot)
    {
        Writer.Write($"&7Opening project at '{projectRoot}'...");
        //Open
        Writer.Write("&2Project opened!");
    }
}