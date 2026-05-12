# Use Ubuntu 22.04
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV PIN_ROOT=/opt/pin

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    python3.10 \
    python3-pip \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Intel Pin 3.28
WORKDIR /opt
RUN wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.28-98749-g6643ecee5-gcc-linux.tar.gz \
    && tar -xzf pin-3.28-98749-g6643ecee5-gcc-linux.tar.gz \
    && mv pin-3.28-98749-g6643ecee5-gcc-linux pin \
    && rm pin-3.28-98749-g6643ecee5-gcc-linux.tar.gz

# Set Environment Variables
ENV PATH="${PATH}:${PIN_ROOT}"

WORKDIR /app

# Presentation Optimization: Install heavy libraries first
# Isse baar-baar build karne par time waste nahi hoga
RUN pip3 install pandas scikit-learn numpy xgboost==2.0.3 streamlit plotly

# 1. First, copy ONLY the patched folder and install it
COPY m2cgen_patched /app/m2cgen_patched
RUN pip3 install ./m2cgen_patched

# 2. Now copy the rest of your code
COPY . .

# Compile the adversarial payload and Pin Tool
RUN g++ beast_target.cpp -o beast_target
RUN cp BranchDataGen.cpp $PIN_ROOT/source/tools/MyPinTool/ \
    && cd $PIN_ROOT/source/tools/MyPinTool \
    && mkdir -p obj-intel64 \
    && make obj-intel64/BranchDataGen.so

EXPOSE 8501

CMD ["streamlit", "run", "app.py", "--server.port=8501", "--server.address=0.0.0.0"]