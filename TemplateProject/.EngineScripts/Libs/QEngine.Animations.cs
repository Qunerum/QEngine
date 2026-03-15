using System;
using System.Collections.Generic;

using QEngine.GUI;

namespace QEngine.Animations
{
    /// <summary>
    /// A resource container that holds a sequence of <see cref="Sprite"/> frames 
    /// and defines the playback speed (FPS).
    /// </summary>
    public class Animation
    {
        List<Sprite> Frames = new();
        int framesPerSecond = 1;
        
        /// <summary> Adds a single <see cref="Sprite"/> to the end of the animation. </summary>
        public void AddFrame(Sprite frame) => Frames.Add(frame);
        /// <summary> Adds a list of <see cref="Sprite"/> frames to the animation. </summary>
        public void AddFrames(List<Sprite> frames) { foreach (var f in frames) Frames.Add(f); }
        /// <summary> 
        /// Retrieves a specific frame by its index. 
        /// Automatically clamps the index to prevent out-of-bounds errors.
        /// </summary>
        public Sprite GetFrame(int index) => Frames[Math.Clamp(index, 0, Frames.Count - 1)];
        /// <summary> Gets the current playback speed in frames per second. </summary>
        public int GetFPS() => framesPerSecond;
        /// <summary> Sets the playback speed in frames per second. </summary>
        public void SetFPS(int fps) => framesPerSecond = fps;
        /// <summary> Returns the full list of sprites in this animation. </summary>
        public List<Sprite> GetFrames() => Frames;
        /// <summary> Gets the total number of frames in this animation. </summary>
        public int GetFramesCount() => Frames.Count;
        /// <summary> Removes all frames from the animation. </summary>
        public void Clear() => Frames.Clear();
        /// <summary> Removes a specific frame by its index. </summary>
        public void RemoveFrame(int index) => Frames.RemoveAt(Math.Clamp(index, 0, Frames.Count - 1));
    }
    
    /// <summary>
    /// A component that manages and plays <see cref="Animation"/> sequences on a <see cref="GameObject"/>.
    /// It controls the playback state and automatically updates the <see cref="Image"/> sprite.
    /// </summary>
    public class Animator : Component
    {
        /// <summary> Gets a value indicating whether the animation is currently paused. </summary>
        public bool isPaused { get; private set; } = false;
        
        Animation? current = null;
        int currentFrame = 0;
        float time = 0;
        
        Dictionary<string, Animation> animations = new();
        
        /// <summary> Retrieves an animation resource by its registered name. </summary>
        /// <param name="name">The name assigned to the animation.</param>
        public Animation GetAnimation(string name) => animations[name];
        
        /// <summary> Creates a new animation entry and adds it to the animator's library. </summary>
        /// <param name="name">The unique name for this animation.</param>
        /// <returns>The newly created <see cref="Animation"/> instance.</returns>
        public Animation AddAnimation(string name) { animations.Add(name, new()); return animations[name]; }
        
        /// <summary> Removes an animation from the library. </summary>
        public void RemoveAnimation(string name) => animations.Remove(name);
        
        /// <summary> 
        /// Adds a single frame to a specific animation in the library. 
        /// </summary>
        /// <param name="name">The name of the target animation.</param>
        /// <param name="frame">The <see cref="Sprite"/> to be added.</param>
        public void AddFrame(string name, Sprite frame) => animations[name].AddFrame(frame);
        
        /// <summary> 
        /// Adds a collection of frames to a specific animation in the library. 
        /// </summary>
        /// <param name="name">The name of the target animation.</param>
        /// <param name="frames">A list of <see cref="Sprite"/> objects.</param>
        public void AddFrames(string name, List<Sprite> frames) => animations[name].AddFrames(frames);
        
        /// <summary> Starts or switches to a specific animation. If the animation is already playing, it continues. </summary>
        public void Play(string name) { if (current != animations[name]) { current = animations[name]; isPaused = false; } }
        
        /// <summary> Pauses the animation at the current frame. </summary>
        public void Pause() => isPaused = true;
        
        /// <summary> Resumes a paused animation. </summary>
        public void UnPause() => isPaused = false;
        
        /// <summary> Resets the animation sequence to the first frame. </summary>
        public void Reset() => currentFrame = 0;
        
        public override void Update()
        {
            if (current == null || isPaused) return;
            time += 1f / 60f;
            float timeToNext = 1f / current.GetFPS();
            while (time >= timeToNext)
            {
                currentFrame++; time -= timeToNext;
                if (currentFrame >= current.GetFrames().Count) { currentFrame = 0; }
                if (gameObject.TryGetComponent(out Image img))
                { img.sprite = current.GetFrame(currentFrame); }
            }
        }
    }
}
