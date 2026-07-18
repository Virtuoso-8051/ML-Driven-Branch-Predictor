import pandas as pd
import numpy as np
import xgboost as xgb
from sklearn.metrics import accuracy_score, confusion_matrix
import time

print("--- BOOTING MEGA AI TRAINING ENGINE (V2 - BENCHMARK EDITION) ---")
total_start_time = time.time()

# 1. Load Data with Aggressive RAM Optimization
print("Loading trace data and optimizing memory...")
dtypes = {
    'IsBackward': 'int8',
    'LocalHistory': 'int16', 
    'Taken': 'int8'
}
df = pd.read_csv("branch_data.csv", dtype=dtypes)

print("Parsing Memory Addresses...")
if df['PC'].dtype == 'object':
    df['PC'] = df['PC'].apply(int, base=16)
if df['Target'].dtype == 'object':
    df['Target'] = df['Target'].apply(int, base=16)

features = ['PC', 'Target', 'IsBackward', 'LocalHistory']
X = df[features]
y = df['Taken']

# 3. STRICT CHRONOLOGICAL SPLIT
print("Performing Chronological Split (80/20)...")
split_index = int(len(df) * 0.8)

X_train = X.iloc[:split_index]
X_test  = X.iloc[split_index:]
y_train = y.iloc[:split_index]
y_test  = y.iloc[split_index:]

# 4. Configure the XGBoost Model
print("Training the XGBoost Ensemble...")
model = xgb.XGBClassifier(
    n_estimators=100,      
    max_depth=6,           
    learning_rate=0.1,     
    tree_method='hist',    
    n_jobs=-1,             
    random_state=42,
    subsample=0.8,         
    colsample_bytree=1.0
)

# --- START TRAINING BENCHMARK ---
train_start = time.perf_counter()
model.fit(X_train, y_train)
train_end = time.perf_counter()
training_time = train_end - train_start
# --- END TRAINING BENCHMARK ---

# 5. Take the Final Exam
print("Testing against the unseen timeline (Future Branches)...")

# --- START INFERENCE BENCHMARK ---
test_start = time.perf_counter()
predictions = model.predict(X_test)
test_end = time.perf_counter()
testing_time = test_end - test_start
# --- END INFERENCE BENCHMARK ---

# Calculate Metrics
accuracy = accuracy_score(y_test, predictions)
total_test_branches = len(y_test)
correct_predictions = sum(y_test == predictions)
incorrect_predictions = total_test_branches - correct_predictions

# Calculate average latency per prediction (in microseconds)
latency_per_branch_us = (testing_time / total_test_branches) * 1_000_000

# Get Confusion Matrix to see Taken vs Not Taken accuracy
tn, fp, fn, tp = confusion_matrix(y_test, predictions).ravel()

print("\n==========================================")
print("  🚀 BENCHMARK RESULTS (DOWNGRADED XGBOOST (version 1.7.3))   ")
print("==========================================")
print(f"Total Execution Time: {time.time() - total_start_time:.2f} seconds")
print(f"Model Training Time (80%):  {training_time:.4f} seconds")
print(f"Model Inference Time (20%): {testing_time:.4f} seconds")
print("------------------------------------------")
print(f"Total Test Branches:      {total_test_branches:,}")
print(f"Correct Predictions:      {correct_predictions:,}")
print(f"Incorrect Predictions:    {incorrect_predictions:,}")
print(f"Final Python Accuracy:    {accuracy * 100:.2f}%")
print("------------------------------------------")
print(f"Average Latency / Branch: {latency_per_branch_us:.4f} µs")
print(f"Actual Taken Correct:     {tp:,}")
print(f"Actual Not Taken Correct: {tn:,}")
print("==========================================\n")

# 6. Save the Brain
print("Saving model logic to branch_predictor_brain_DOWNGRADED.json...")
model.save_model("branch_predictor_brain_DOWNGRADED.json")
print("Done! Ready for Phase 3: m2cgen transpilation.")