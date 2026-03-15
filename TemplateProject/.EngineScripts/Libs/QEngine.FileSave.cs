using System.IO;

namespace QEngine.FileSave
{
    /// <summary> 
    /// Utility class for handling file I/O operations. 
    /// Supports the ";here;" shorthand to target the local Assets directory.
    /// </summary>
    public static class FileSave
    {
        /// <summary>  Writes the specified string to a file. Replaces ";here;" with the Assets path. </summary>
        /// <param name="path">The file path (can include ";here;").</param>
        /// <param name="content">The string content to write.</param>
        public static void Write(string path, string? content) 
            => File.WriteAllText(path.Replace(";here;", Assets.assetsPath), content);
        /// <summary>  Reads the content of a file as a string. Replaces ";here;" with the Assets path. </summary>
        /// <param name="path">The file path (can include ";here;").</param>
        /// <returns>The content of the file.</returns>
        public static string Read(string path) 
            => File.ReadAllText(path.Replace(";here;", Assets.assetsPath));
        /// <summary> Checks if a file exists at the specified path. </summary>
        public static bool Exists(string path) 
            => File.Exists(path.Replace(";here;", Assets.assetsPath));
    }
}
