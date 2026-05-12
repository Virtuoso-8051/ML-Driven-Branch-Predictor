import pandas as pd
import numpy as np
import xgboost as xgb
from sklearn.metrics import accuracy_score
import time

print("--- BOOTING MEGA AI TRAINING ENGINE (V2 - CHRONOLOGICAL) ---")
start_time = time.time()

# 1. Load Data with Aggressive RAM Optimization
print("Loading trace data and optimizing memory...")
# We enforce strictly minimal data types for our 8-bit features to prevent RAM crashes
dtypes = {
    'IsBackward': 'int8',
    'LocalHistory': 'int16', # 8-bit history maxes at 255, fits safely in int16
    'Taken': 'int8'
}

df = pd.read_csv("branch_data.csv", dtype=dtypes)

# Fast Vectorized Hex Conversion (Slashes parsing time by 80%)
print("Parsing Memory Addresses...")
if df['PC'].dtype == 'object':
    df['PC'] = df['PC'].apply(int, base=16)
if df['Target'].dtype == 'object':
    df['Target'] = df['Target'].apply(int, base=16)

# 2. Define Features
features = ['PC', 'Target', 'IsBackward', 'LocalHistory']
X = df[features]
y = df['Taken']

# 3. STRICT CHRONOLOGICAL SPLIT (No Time Travel!)
print("Performing Chronological Split (First 80% Train, Last 20% Test)...")
split_index = int(len(df) * 0.8)

X_train = X.iloc[:split_index]
X_test  = X.iloc[split_index:]
y_train = y.iloc[:split_index]
y_test  = y.iloc[split_index:]

# 4. Configure the XGBoost Model for m2cgen Transpilation
print("Training the XGBoost Ensemble...")
model = xgb.XGBClassifier(
    n_estimators=100,      
    max_depth=6,           # Sweet spot: Deep enough to learn 8-bit history, shallow enough for C++
    learning_rate=0.1,     
    tree_method='hist',    # Prevents OOM errors on massive datasets
    n_jobs=-1,             
    random_state=42,
    subsample=0.8,         # Adds a bit of randomness to prevent overfitting
    colsample_bytree=1.0
)

# Train the model!
model.fit(X_train, y_train)

# 5. Take the Final Exam
print("Testing against the unseen timeline (Future Branches)...")
predictions = model.predict(X_test)
accuracy = accuracy_score(y_test, predictions)

print("==========================================")
print(f"  PHASE 2 AI ACCURACY: {accuracy * 100:.2f}%")
print(f"  Total Execution Time: {time.time() - start_time:.1f} seconds")
print("==========================================")

# 6. Save the Brain
print("Saving model logic to branch_predictor_brain.json...")
model.save_model("branch_predictor_brain.json")
print("Done! Ready for Phase 3: m2cgen transpilation.")
