// -------------------------
// Komendy
// -------------------------

using QEConsole;

    class NewProjectCommand : ICommand
    {
        public string Name => "new";
        public string Description => "Create new project: qe new project <name>";
        public bool RequiresProject => false;

        public void Execute(string[] args, string? _)
        {
            if (args.Length < 2 || args[0] != "project")
            {
                Console.WriteLine("Usage: qe new project <name>");
                return;
            }

            string name = args[1];
            Directory.CreateDirectory(name);
            File.WriteAllText(Path.Combine(name, QEngineData.projFile),
                "{\n" +
                $"    \"name\": \"{name}\",\n" +
                $"    \"engineVersion\": \"{QEngineData.version}\"\n" +
                "}");

            Console.WriteLine($"Project '{name}' created!");
        }
    }

    class BuildCommand : ICommand
    {
        public string Name => "build";
        public string Description => "Build current project";
        public bool RequiresProject => true;

        public void Execute(string[] args, string? projectRoot)
        {
            Console.WriteLine($"Building project at '{projectRoot}'...");
            // Tutaj dodajesz prawdziwy build
            Console.WriteLine("Build finished!");
        }
    }

    class RunCommand : ICommand
    {
        public string Name => "run";
        public string Description => "Run current project";
        public bool RequiresProject => true;

        public void Execute(string[] args, string? projectRoot)
        {
            Console.WriteLine($"Running project at '{projectRoot}'...");
            // Tutaj odpala się Twoja gra
            Console.WriteLine("Game is running!");
        }
    }

    class BuildRunCommand : ICommand
    {
        public string Name => "buildrun";
        public string Description => "Build and run current project";
        public bool RequiresProject => true;

        public void Execute(string[] args, string? projectRoot)
        {
            var build = new BuildCommand();
            var run = new RunCommand();
            build.Execute(args, projectRoot);
            run.Execute(args, projectRoot);
        }
    }