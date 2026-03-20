using System;
using QEngine.GUI;
// ReSharper disable All

namespace QEngine.Mathematics
{
    /// <summary> 
    /// Specialized math utility class for engine-specific calculations. 
    /// Focuses on interpolation, remapping, and coordinate conversions.
    /// </summary>
    public static class QMath
    {
        /// <summary> Maps a value from one range [inMin, inMax] to another [outMin, outMax]. </summary>
        public static float Remap(float value, float inMin, float inMax, float outMin, float outMax) => (value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
        /// <summary> Rounds a float value to a specified number of decimal places. </summary>
        public static float Round(float value, int decimals) => MathF.Round(value, decimals);
        /// <summary> Rounds both components of a Vector2 to a specified number of decimal places. </summary>
        public static Vector2 Round(Vector2 value, int decimals) => new(Round(value.x, decimals), Round(value.y, decimals));
        /// <summary> Converts an angle in degrees to a normalized direction vector. </summary>
        public static Vector2 AngleToVector(float angleDeg)
        {
            float rad = angleDeg * MathF.PI / 180f, x = MathF.Cos(rad), y = MathF.Sin(rad);
            return new (x, y);
        }
        
        #region Lerps
        /// <summary> Linearly interpolates between two floats with precision clamping. </summary>
        public static float Lerp(float a, float b, float t, int decimals = 4) => 
            Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        /// <summary> Linearly interpolates between two Vector2 positions. </summary>
        public static Vector2 Lerp(Vector2 a, Vector2 b, float t, int decimals = 4) => 
            Round(a + (b - a) * Math.Clamp(t, 0, 1), decimals);
        /// <summary> Linearly interpolates between two Colors, clamping result to 0-255 byte range. </summary>
        public static Color Lerp(Color a, Color b, float t)
        => new((byte)Math.Clamp(a.r + (b.r - a.r) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.g + (b.g - a.g) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.b + (b.b - a.b) * Math.Clamp(t, 0, 1), 0, 255),
                (byte)Math.Clamp(a.a + (b.a - a.a) * Math.Clamp(t, 0, 1), 0, 255));
        #endregion
    }
}
