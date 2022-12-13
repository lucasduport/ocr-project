<h1>OCR Project | S3 Epita</h1>
Neural network is not working but you can try UI with ImageProcessing/Images folder
<p>This is our third semester project at EPITA. You can find the subject
<a href="https://cdn.discordapp.com/attachments/1002568563892166838/1037486340301918240/ocr_sudoku_solver_fr.pdf">here</a>.</p>

<h2>Getting Started</h2>

<p>These instructions will give you a copy of the project up and running on
your local machine for development and testing purposes. See deployment
for notes on deploying the project on a live system.</p>

<h3>Prerequisites</h3>

<p>Requirements for the software and other tools to build, and test: <br/>
- <a href="https://clang.llvm.org/">Clang</a> <br/>
- <a href="https://www.libsdl.org/">Simple DirectMedia Layer library (SDL)</a> <br/>
- <a href="https://www.gtk.org/">The GIMP Toolkit (GTK)</a></p>

<h3>Installing</h3>

<p>A step by step series of examples that tell you how to get a development
environment running</p>

<p>First, you have to install clang compiler</p>

<pre><code>sudo apt install clang
</code></pre>

<p>Then, you have to install SDL2 library</p>

<pre><code>sudo apt install libsdl2-dev
</code></pre>

<p>Now, you have to install SDL2 image library</p>

<pre><code>sudo apt install libsdl2-image-dev
</code></pre>

<p>Now, you have to install GTK library</p>
sudo apt-get install libgtk-3-dev

<pre><code>sudo apt-get install libgtk-3-dev
</code></pre>

<p>Finally, you have all the tools needed for the project.</p>

<h3>Correct indentation and coding style:</h3>

<pre><code>clang-format --style=file:.clang-format -i *.c
</code></pre>

<h2>Final/</h2>

<h3>Build</h3>

<pre><code>make
</code></pre>

<h3>Run</h3>

<pre><code>./main
</code></pre>

<h2>NeuralNetwork/</h2>

<h3>Build</h3>

<p>Build for release</p>

<pre><code>make
</code></pre>

<p>Build for debug</p>

<pre><code>make debug
</code></pre>

<p>Clean build files</p>

<pre><code>make clean
</code></pre>

<p>Clean build files and executable</p>

<pre><code>make clear
</code></pre>

<ul>
<li><p>Training:</p>

<pre><code>./NeuralNetwork Train [Config file (*.cfg)] [Training data (.bin)] [Validation data (.bin)] [Number of attempt] [Transfer Network (.dnn)]
</code></pre></li>
</ul>

<h3>Examples:</h3>

<p>Example of train with 1 attempt:</p>

<pre><code>ex: ./NeuralNetwork Train TrainedNetwork/NN.cfg DataSets/hcd_784_135_validation.bin DataSets/hcd_784_135_validation.bin 1
</code></pre>

<h2>ImageProcessing/</h2>

<h3>Build</h3>

<p>Build for release</p>

<pre><code>make
</code></pre>

<h3>Run</h3>

<p>Try --help or -h for more information</p>

<pre><code>./main --help
</code></pre>

<h3>Examples:</h3>

<pre><code>./main -d &lt;ImagePath&gt; # Demonstration
./main -r &lt;ImagePath&gt; &lt;Angle&gt; # Rotate image with angle
</code></pre>

<h2>Solver/</h2>

<h3>Build</h3>

<p>Build for release</p>

<pre><code>make
</code></pre>

<h3>Run</h3>

<pre><code>./solver &lt;Textfile&gt;
</code></pre>

<h3>Example:</h3>

<pre><code>./solver &lt;Textfile&gt; # Solve sudoku which is in textfile
</code></pre>

<h2>Authors</h2>

<ul>
<li>Mattéo BAUSSART - 
<a href="matteo.baussart@epita.fr">matteo.baussart@epita.fr</a></li>
<li>Lucas DUPORT - 
<a href="lucas.duport@epita.fr">lucas.duport@epita.fr</a></li>
<li>Julie BLASSIAU - 
<a href="julie.blassiau@epita.fr">julie.blassiau@epita.fr</a></li>
<li>Matthieu CORREIA - 
<a href="matthieu.correia@epita.fr">matthieu.correia@epita.fr</a></li>
</ul>
