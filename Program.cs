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
                Console.WriteLine($"Unknown command: {name}");
                return;
            }

            if (cmd.RequiresProject && projectRoot == null)
            {
                Console.WriteLine("Error: not a QE project directory (qe.project.json not found).");
                return;
            }

            cmd.Execute(args, projectRoot);
        }

        public void ListCommands()
        {
            foreach (var c in commands.Values)
                Console.WriteLine($"{c.Name} - {c.Description}");
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
                if (File.Exists(Path.Combine(dir.FullName, "qe.project.json")))
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
        public static string version = "0.1.0";
        public static string projFile = ".qeproject";
    }
    
    class Program
    {
        static void Main(string[] args)
        {
            var registry = new CommandRegistry();

            // Rejestracja komend
            registry.Register(new NewProjectCommand());
            registry.Register(new BuildCommand());
            registry.Register(new RunCommand());
            registry.Register(new BuildRunCommand());

            // Alias bar -> buildrun
            registry.Alias("bar", "buildrun");

            if (args.Length == 0)
            {
                Console.WriteLine($"= = = > QEngine {QEngineData.version}");
                Console.WriteLine("= > Available commands:");
                registry.ListCommands();
                return;
            }

            var commandName = args[0];
            var commandArgs = args.Skip(1).ToArray();
            var projectRoot = ProjectHelper.FindProjectRoot();

            registry.Execute(commandName, commandArgs, projectRoot);
        }
    }
}