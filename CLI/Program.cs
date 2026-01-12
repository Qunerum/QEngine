namespace QEConsole
{
    // -------------------------
    // Interfejs komendy
    // -------------------------
    public interface ICommand
    {
        string Name { get; }
        string Description { get; }
        bool RequiresProject { get; }
        void Execute(string[] args, string? projectRoot);
    }

    // -------------------------
    // Rejestr komend
    // -------------------------
    class CommandRegistry
    {
        private readonly Dictionary<string, ICommand> commands = new();
        private readonly Dictionary<string, string> aliases = new();

        public void Register(ICommand command) => commands[command.Name] = command;
        public void Alias(string alias, string target) => aliases[alias] = target;

        public void Execute(string name, string[] args, string? projectRoot)
        {
            if (aliases.TryGetValue(name, out var real)) name = real;

            if (!commands.TryGetValue(name, out var cmd))
            {
                Writer.Write($"&4Unknown command: &6{name}");
                return;
            }

            if (cmd.RequiresProject && projectRoot == null)
            {
                Writer.Write($"&4Error: not a &eQEngine &4project directory ({QEngineData.projFile} not found).");
                return;
            }

            cmd.Execute(args, projectRoot);
        }

        public void ListCommands()
        {
            foreach (var c in commands.Values)
                Writer.Write($" &2> &e{c.Name} &7- {c.Description}");
        }
    }

    // -------------------------
    // Funkcja szukania projektu
    // -------------------------
    static class ProjectHelper
    {
        public static string? FindProjectRoot()
        {
            var dir = new DirectoryInfo(Directory.GetCurrentDirectory());
            while (dir != null)
            {
                if (File.Exists(Path.Combine(dir.FullName, QEngineData.projFile)))
                    return dir.FullName;
                dir = dir.Parent;
            }
            return null;
        }
    }

    // -------------------------
    // Program główny
    // -------------------------
    public static class QEngineData
    {
        public static string version = "0.2.1";
        public static string projFile = ".qeproject";
    }
    
    class Program
    {
        static void Main(string[] args)
        {
            var registry = new CommandRegistry();

            // Rejestracja komend
            registry.Register(new NewProjectCommand());
            registry.Register(new RunCommand());
            registry.Register(new BuildCommand());
            registry.Register(new BARCommand());
            registry.Register(new SearchCommand());
            registry.Register(new UpdateCommand());

            // Alias bar -> buildrun
            registry.Alias("bar", "buildrun");

            if (args.Length == 0)
            {
                Console.Clear();
                Writer.Write($"&2= = = > &eQEngine &7{QEngineData.version}");
                Writer.Write("&2= = > &6Available commands:");
                registry.ListCommands();
                Console.WriteLine();
                return;
            }

            var commandName = args[0];
            var commandArgs = args.Skip(1).ToArray();
            var projectRoot = ProjectHelper.FindProjectRoot();

            registry.Execute(commandName, commandArgs, projectRoot);
        }
    }

    public static class Writer
    {
        static Dictionary<char, ConsoleColor> clrs = new()
        {
            { '0', ConsoleColor.Black }, { '8', ConsoleColor.Gray },
            { '1', ConsoleColor.DarkBlue }, { '9', ConsoleColor.Blue },
            { '2', ConsoleColor.DarkGreen }, { 'a', ConsoleColor.Green },
            { '3', ConsoleColor.DarkCyan }, { 'b', ConsoleColor.Cyan },
            { '4', ConsoleColor.DarkRed }, { 'c', ConsoleColor.Red },
            { '5', ConsoleColor.DarkMagenta }, { 'd', ConsoleColor.Magenta },
            { '6', ConsoleColor.DarkYellow }, { 'e', ConsoleColor.Yellow },
            { '7', ConsoleColor.DarkGray }, { 'f', ConsoleColor.White },
        };
        public static void Write(string msg)
        {
            string[] ws = msg.Split('&');
            foreach (var w in ws)
            {
                if (string.IsNullOrEmpty(w))
                    continue;
                if (clrs.TryGetValue(w[0], out var clr))
                {
                    Console.ForegroundColor = clr;
                    if (w.Length > 1)
                        Console.Write(w[1..]);
                }
                else { Console.Write(w); }
            }
            Console.WriteLine();
            Console.ResetColor();
        }
    }

    public static class FileManager
    {
        public static void CopyDirectory(string sourceDir, string targetDir)
        {
            Directory.CreateDirectory(targetDir);

            foreach (var file in Directory.GetFiles(sourceDir))
                File.Copy(file, Path.Combine(targetDir, Path.GetFileName(file)), true);

            foreach (var dir in Directory.GetDirectories(sourceDir))
                CopyDirectory(dir, Path.Combine(targetDir, Path.GetFileName(dir)));
        }
        public static List<string> GetSearchRoots()
        {
            var home = Environment.GetFolderPath(
                Environment.SpecialFolder.UserProfile);

            var roots = new List<string>
            {
                home,
                Path.Combine(home, "Documents"),
                Path.Combine(home, "Projects"),
                Path.Combine(home, "Dev"),
                Path.Combine(home, "Games")
            };

            return roots;
        }

    }
}