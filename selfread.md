1. made folder "mkdir ~/ml_branch_predictor 
2. cd into the folder "cd ~/ml_branch_predictor"
3. compile the target "g++ beast_target.cpp -o beast_target"
4. Move and Compile the Pin Tool
"cp BranchDataGen.cpp $PIN_ROOT/source/tools/MyPinTool/
cd $PIN_ROOT/source/tools/MyPinTool
rm -f obj-intel64/BranchDataGen.so
make obj-intel64/BranchDataGen.so"
5. run the extraction
"cd ~/ml_branch_predictor
$PIN_ROOT/pin -t $PIN_ROOT/source/tools/MyPinTool/obj-intel64/BranchDataGen.so -- ./beast_target"
6. this will display the first 15 lines of the generated branch_data.csv file
"head -n 15 branch_data.csv" [optional]
7. build 2-bit counter which mimics what intel CPU does in L1 cache,"test_2bit.cpp"
"g++ test_2bit.cpp -o test_2bit
./test_2bit"

# ML training starts here:
1. step back to home directory "cd ~"
2. download the linux installer from web
"wget https://repo.anaconda.com/archive/Anaconda3-2024.05-Linux-x86_64.sh"
3. run the installer "bash Anaconda3-2024.05-Linux-x86_64.sh"
4. follow the instructions to complete the installation 
   Read the prompts carefully:
    ->Press Enter to view the license.
    ->Hold the Spacebar or Enter to scroll to the bottom.
    ->Type yes and press Enter to accept the terms.
    ->Press Enter to confirm the default installation location.
    ->CRITICAL: When it asks if you want the installer to initialize Miniconda3 by running conda init, type yes and press Enter!
5. refresh the terminal "source ~/.bashrc" to recognize the conda command
6. if error occurs for terms & conds, accept those by running these commands (these are exact links appeared in the error message, so copy and paste them as is):
"conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main
conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r"
6. run the following commands to set up the conda environment and install the necessary libraries for ML training
"cd ~/ml_branch_predictor
conda create -n ml_branch python=3.10 -y
conda activate ml_branch
conda install pandas scikit-learn numpy -y"

7. install xgboost for training the model "conda install xgboost -y"
8. "train_model.py" -> this just take 100,000 rows of the generated branch_data.csv file and just predict the accuracy using a decision tree.

9. "train_xgboost.py" -> 
The XGBoost Way (Sequential Trees + Parallel Splitting)
XGBoost stands for Extreme Gradient Boosting. Boosting is sequential.

Tree 1 makes a prediction on the data. It gets some right, and some wrong.

Tree 2 looks only at the mistakes Tree 1 made, and tries to fix them.

Tree 3 looks at the mistakes Tree 2 made, and tries to fix those.
It builds them one by one, learning from the previous errors.

So why did all 8 of your CPU cores light up?
While the trees are built one after another, the math to build a single tree is brutally heavy. To decide exactly where to split a branch (e.g., "Is PC > 0x4000?"), the algorithm has to scan all 1,000,000 rows.

XGBoost uses your 8 cores to parallelize the scanning and math. It chops the 1 million rows into blocks, hands a block to every core, and they all crunch the math simultaneously to build that single tree in milliseconds. Then, they all work together on the next tree.

10. "train_final.py"
We are removing the nrows limit. Your laptop is about to load and process all 31+ million rows (665MB).

Because your 16GB of RAM is going to be pushed to the absolute limit during the Feature Engineering phase, I have added a secret weapon to this code: gc.collect(). This forces Python's "Garbage Collector" to instantly flush dead data out of your RAM before the model starts training, saving your laptop from crashing.

:-Close EVERYTHING. Chrome, Discord, background apps. Leave only VS Code open.

11. add these in train_final.py -> this exports the trained model to SSD as a JSON file, which can be loaded later for making predictions without needing to retrain the model.
"print("\n -> Exporting trained model to disk...")
model.save_model("branch_predictor_brain.json")
print(" -> Model saved successfully!")"

# use trained model for predictions
1. install the traspiler m2cgen(model-to-code generator) "pip install m2cgen"
2. Write the Brain Extractor Script "extract_brain.py" -> this loads the trained model from disk, and uses m2cgen to convert the model into massive block of pure, hardcoded C++ if-else statements that represent the logic of the XGBoost trees.
3. [CORRECTION]
m2cgen expects pure numbers (floats) whereas the new XGBoost 2.0+ stores them as lists, so we need to do "VERSION PINNING" of xgboost to 1.7.6, which is the last version that still stores vals as pure numbers. Run this command to install the specific version:
3a. "conda remove xgboost -y" -> "pip uninstall xgboost -y" -> "rm /home/reverie/miniconda3/envs/ml_branch/lib/libxgboost.so" -> conda remove libxgboost py-xgboost -y" -> "pip install xgboost==1.7.6" 
3b. "python train_final.py" -> "extract_brain.py" -> will create ""ai_predictor.h""

4. create "trace_simulator.cpp" -> this is a C++ program that simulates the branch predictor using the generated "ai_predictor.h" and evaluates its accuracy against the original branch_data.csv. It reads each row, feeds the features into the hardcoded if-else logic, and compares the prediction to the actual outcome, tallying the results to compute the final accuracy of the AI-powered branch predictor.

5. compile & run 
"g++ trace_simulator.cpp -o trace_sim -O3
./trace_sim"