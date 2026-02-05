using System.IO;

namespace QEngine.FileSave
{
    public static class FileSave
    {
        public static void Write(string path, string? content) 
            => File.WriteAllText(path.Replace(";here;", Path.Combine(Directory.GetCurrentDirectory(), "Assets")), content);
        public static string Read(string path) 
            => File.ReadAllText(path.Replace(";here;", Path.Combine(Directory.GetCurrentDirectory(), "Assets")));
    }
}
