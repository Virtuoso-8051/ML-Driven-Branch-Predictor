import streamlit as st
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from collections import defaultdict
import subprocess
import tempfile
import os
import shutil
import xgboost as xgb

st.set_page_config(
    page_title="ML Branch Predictor",
    page_icon="🧠",
    layout="wide",
    initial_sidebar_state="expanded"
)

st.markdown("""
<style>
    @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;700&family=Space+Grotesk:wght@300;400;500;600;700&display=swap');
    html, body, [class*="css"] { font-family: 'Space Grotesk', sans-serif; }
    .stApp { background: #0a0a0f; color: #e2e8f0; }
    .main-header {
        font-family: 'JetBrains Mono', monospace;
        font-size: 2.4rem; font-weight: 700; color: #00ff88;
        letter-spacing: -1px; margin-bottom: 0;
        text-shadow: 0 0 30px rgba(0,255,136,0.3);
    }
    .sub-header {
        font-family: 'JetBrains Mono', monospace;
        font-size: 0.85rem; color: #4a5568;
        letter-spacing: 2px; text-transform: uppercase; margin-bottom: 2rem;
    }
    .metric-card {
        background: #111118; border: 1px solid #1e2030;
        border-radius: 12px; padding: 1.2rem 1.5rem;
        text-align: center; position: relative; overflow: hidden;
    }
    .metric-card::before {
        content: ''; position: absolute; top: 0; left: 0; right: 0;
        height: 2px; background: linear-gradient(90deg, #00ff88, #0088ff);
    }
    .metric-value {
        font-family: 'JetBrains Mono', monospace;
        font-size: 2.2rem; font-weight: 700; color: #00ff88;
    }
    .metric-label {
        font-size: 0.75rem; color: #4a5568;
        text-transform: uppercase; letter-spacing: 1.5px; margin-top: 4px;
    }
    .section-title {
        font-family: 'JetBrains Mono', monospace; font-size: 0.8rem;
        color: #00ff88; text-transform: uppercase; letter-spacing: 3px;
        margin-bottom: 1rem; padding-bottom: 0.5rem; border-bottom: 1px solid #1e2030;
    }
    .stButton > button {
        background: linear-gradient(135deg, #00ff88 0%, #0088ff 100%);
        color: #0a0a0f; border: none; border-radius: 8px;
        font-family: 'JetBrains Mono', monospace; font-weight: 700;
        font-size: 0.85rem; letter-spacing: 1px;
        padding: 0.6rem 1.5rem; width: 100%;
    }
    .result-box {
        background: #111118; border: 1px solid #1e2030; border-radius: 12px;
        padding: 1.5rem; font-family: 'JetBrains Mono', monospace;
        font-size: 0.85rem; color: #a0aec0; line-height: 1.8;
    }
    .highlight-green { color: #00ff88; font-weight: 700; }
    .highlight-blue  { color: #0088ff; font-weight: 700; }
    .highlight-orange{ color: #ff8800; font-weight: 700; }
    div[data-testid="stSidebar"] { background: #080810; border-right: 1px solid #1e2030; }
    .stTabs [data-baseweb="tab-list"] { background: #111118; border-radius: 10px; padding: 4px; gap: 4px; }
    .stTabs [data-baseweb="tab"] { font-family: 'JetBrains Mono', monospace; font-size: 0.8rem; letter-spacing: 1px; color: #4a5568; border-radius: 8px; }
    .stTabs [aria-selected="true"] { background: #1e2030 !important; color: #00ff88 !important; }
</style>
""", unsafe_allow_html=True)

# ── constants ──────────────────────────────────────────────────────────────────
DARK   = '#0a0a0f'
CARD   = '#111118'
GREEN  = '#00ff88'
BLUE   = '#0088ff'
ORANGE = '#ff8800'
GRID   = '#1e2030'

PLOTLY_LAYOUT = dict(
    paper_bgcolor=DARK, plot_bgcolor=CARD,
    font=dict(family='JetBrains Mono', color='#a0aec0', size=11),
    margin=dict(l=40, r=20, t=40, b=40),
)

# --- MEMORY-OPTIMIZED CSV LOADER ---
@st.cache_data
def load_csv(file):
    dtypes = {'IsBackward': 'int8', 'LocalHistory': 'int16', 'Taken': 'int8'}
    df = pd.read_csv(file, dtype=dtypes)
    if df['PC'].dtype == 'object':
        df['PC'] = df['PC'].apply(lambda x: int(str(x), 16))
    if df['Target'].dtype == 'object':
        df['Target'] = df['Target'].apply(lambda x: int(str(x), 16))
    return df

# --- THE REAL XGBOOST BRAIN ---
def run_ml_predictor(df):
    model_path = "branch_predictor_brain.json"
    if not os.path.exists(model_path):
        st.error("JSON Brain not found! Please run train_final.py")
        return 0, pd.DataFrame()
        
    model = xgb.XGBClassifier()
    model.load_model(model_path)
    
    features = ['PC', 'Target', 'IsBackward', 'LocalHistory']
    X = df[features]
    actuals = df['Taken'].values
    
    predictions = model.predict(X)
    correct_array = (predictions == actuals)
    
    # Aditi's UI needs this specific DataFrame structure
    rows = pd.DataFrame({
        "predicted": predictions,
        "actual": actuals,
        "correct": correct_array,
        "pc": df['PC']
    })
    
    return correct_array.mean() * 100, rows

# --- TRUE HARDWARE 2-BIT SIMULATOR ---
def run_2bit_predictor(df):
    BHT_ENTRIES = 8192
    BHT_MASK = BHT_ENTRIES - 1
    bht = np.ones(BHT_ENTRIES, dtype=np.int8)
    
    predictions = np.zeros(len(df), dtype=np.int8)
    actuals = df['Taken'].values
    pcs = df['PC'].values
    
    for i in range(len(df)):
        index = pcs[i] & BHT_MASK
        state = bht[index]
        pred = 1 if state >= 2 else 0
        predictions[i] = pred
        
        if actuals[i] == 1:
            if state < 3: bht[index] += 1
        else:
            if state > 0: bht[index] -= 1
            
    correct_array = (predictions == actuals)
    
    rows = pd.DataFrame({
        "predicted": predictions,
        "actual": actuals,
        "correct": correct_array,
        "pc": pcs
    })
    
    return correct_array.mean() * 100, rows

def run_all(df):
    ml_acc,  ml_preds  = run_ml_predictor(df)
    tbt_acc, tbt_preds = run_2bit_predictor(df)
    st.session_state.update({
        "df": df, "ml_preds": ml_preds, "tbt_preds": tbt_preds,
        "ml_acc": ml_acc, "tbt_acc": tbt_acc, "ready": True
    })

def result_box(ml_acc, tbt_acc, n_rows):
    diff   = abs(ml_acc - tbt_acc)
    winner = "ML" if ml_acc >= tbt_acc else "2-bit"
    st.markdown(f"""
    <div class="result-box">
        ┌──────────────────────────────────┐<br>
        │  SIMULATION COMPLETE             │<br>
        ├──────────────────────────────────┤<br>
        │  Branches : <span class="highlight-green">{n_rows:,}</span><br>
        │  ML acc   : <span class="highlight-green">{ml_acc:.2f}%</span><br>
        │  2-bit acc: <span class="highlight-blue">{tbt_acc:.2f}%</span><br>
        │  Diff     : <span class="highlight-orange">{diff:.2f}%</span><br>
        │  Winner   : <span class="highlight-green">{winner}</span><br>
        └──────────────────────────────────┘
    </div>""", unsafe_allow_html=True)
    fig = go.Figure()
    for val, name, color in [(ml_acc,"ML Model",GREEN),(tbt_acc,"2-bit",BLUE)]:
        fig.add_trace(go.Bar(x=[name], y=[val], marker_color=color, width=0.35,
            text=[f"{val:.2f}%"], textposition="outside",
            textfont=dict(family="JetBrains Mono", color=color, size=13)))
    fig.update_layout(**PLOTLY_LAYOUT, height=260,
        yaxis=dict(range=[70,100],gridcolor=GRID), showlegend=False, bargap=0.5)
    st.plotly_chart(fig, use_container_width=True)

# ── sidebar ────────────────────────────────────────────────────────────────────
with st.sidebar:
    st.markdown('<div class="section-title">// Navigation</div>', unsafe_allow_html=True)
    page = st.radio("", ["📁  Upload CSV", "📊  Visualizations"], label_visibility="collapsed")
    st.markdown("---")
    if st.session_state.get("ready"):
        ml_acc  = st.session_state["ml_acc"]
        tbt_acc = st.session_state["tbt_acc"]
        st.markdown(f"""
        <div style="font-family:'JetBrains Mono',monospace;font-size:0.75rem;line-height:1.9;">
        <span style="color:#4a5568;">// Last run</span><br>
        ML  : <span style="color:{GREEN}">{ml_acc:.2f}%</span><br>
        2bit: <span style="color:{BLUE}">{tbt_acc:.2f}%</span>
        </div>""", unsafe_allow_html=True)


# ══════════════════════════════════════════════════════════════════════════════
# PAGE 1 — Upload CSV
# ══════════════════════════════════════════════════════════════════════════════
if "Upload" in page:
    st.markdown('<div class="main-header">Upload Trace</div>', unsafe_allow_html=True)
    st.markdown('<div class="sub-header">// Upload any branch_data.csv from Intel Pin</div>', unsafe_allow_html=True)

    uploaded = st.file_uploader("Drop your branch_data.csv here", type=["csv"])
    if uploaded:
        with st.spinner("Loading..."):
            df = load_csv(uploaded)

        c1, c2, c3 = st.columns(3)
        for col, val, label in zip([c1,c2,c3],
            [f"{len(df):,}", f"{df['PC'].nunique():,}", f"{df['Taken'].mean()*100:.1f}%"],
            ["Total branches","Unique PCs","Branches taken"]):
            col.markdown(f'<div class="metric-card"><div class="metric-value">{val}</div><div class="metric-label">{label}</div></div>', unsafe_allow_html=True)

        st.markdown("<br>", unsafe_allow_html=True)

        if st.button("▶  RUN BOTH PREDICTORS"):
            with st.spinner("Running XGBoost and 2-Bit Hardware Simulators..."):
                run_all(df)
            result_box(st.session_state["ml_acc"], st.session_state["tbt_acc"], len(df))
            st.success("✓ Done! Switch to Visualizations in the sidebar.")


# ══════════════════════════════════════════════════════════════════════════════
# PAGE 2 — Visualizations
# ══════════════════════════════════════════════════════════════════════════════
elif "Visual" in page:
    st.markdown('<div class="main-header">Visualizations</div>', unsafe_allow_html=True)
    st.markdown('<div class="sub-header">// Branch behavior analysis</div>', unsafe_allow_html=True)

    if not st.session_state.get("ready"):
        st.warning("Run a prediction first using **Upload CSV**.")
        st.stop()

    df        = st.session_state["df"]
    ml_preds  = st.session_state["ml_preds"]
    tbt_preds = st.session_state["tbt_preds"]

    tab1, tab2, tab3 = st.tabs(["Rolling Accuracy", "Distribution", "Hot Branches"])

    with tab1:
        window = st.slider("Rolling window", 500, 20000, 5000, step=500)
        ml_r   = ml_preds["correct"].rolling(window).mean() * 100
        tbt_r  = tbt_preds["correct"].rolling(window).mean() * 100
        
        max_points = 10000
        step = max(1, len(ml_r) // max_points)
        ml_r_plot  = ml_r.iloc[::step]
        tbt_r_plot = tbt_r.iloc[::step]
        x = list(range(len(ml_r_plot)))

        fig = go.Figure()
        fig.add_trace(go.Scatter(x=x, y=ml_r_plot, name="ML Model", line=dict(color=GREEN, width=1.5)))
        fig.add_trace(go.Scatter(x=x, y=tbt_r_plot, name="2-bit", line=dict(color=BLUE, width=1.5, dash="dot")))
        fig.update_layout(**PLOTLY_LAYOUT, height=400, yaxis=dict(range=[40,105],gridcolor=GRID))
        st.plotly_chart(fig, use_container_width=True)

    with tab2:
        taken = int(df["Taken"].sum())
        not_t = len(df)-taken
        fig = go.Figure(go.Pie(labels=["Taken","Not Taken"], values=[taken, not_t], hole=0.65, marker=dict(colors=[GREEN,BLUE])))
        fig.update_layout(**PLOTLY_LAYOUT, height=400)
        st.plotly_chart(fig, use_container_width=True)

    with tab3:
        pc_counts = df.groupby("PC")["Taken"].count().sort_values(ascending=False).head(15)
        pc_counts.index = [hex(i) for i in pc_counts.index]
        table_df = pd.DataFrame({"Branch Address (PC)": pc_counts.index, "Execution Count": pc_counts.values})
        st.dataframe(table_df, use_container_width=True, hide_index=True)