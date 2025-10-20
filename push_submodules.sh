#!/bin/bash

# Script to push both InverseModeling and Utilities submodules

echo "====================================="
echo "Pushing Submodules"
echo "====================================="

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to push a submodule
push_submodule() {
    local submodule_path=$1
    local submodule_name=$2
    
    echo ""
    echo -e "${YELLOW}Processing ${submodule_name}...${NC}"
    
    if [ ! -d "$submodule_path" ]; then
        echo -e "${RED}Error: ${submodule_path} does not exist${NC}"
        return 1
    fi
    
    cd "$submodule_path"
    
    # Check if on a branch (not detached HEAD)
    current_branch=$(git symbolic-ref --short HEAD 2>/dev/null)
    
    if [ -z "$current_branch" ]; then
        echo -e "${RED}Error: Detached HEAD state. Checking out main...${NC}"
        git checkout main
        if [ $? -ne 0 ]; then
            echo -e "${RED}Failed to checkout main branch${NC}"
            cd - > /dev/null
            return 1
        fi
        current_branch="main"
    fi
    
    echo "Current branch: ${current_branch}"
    
    # Check if there are changes to commit
    if ! git diff-index --quiet HEAD --; then
        echo -e "${YELLOW}Uncommitted changes found. Committing...${NC}"
        git add .
        git commit -m "Auto-commit before push"
    fi
    
    # Push
    echo "Pushing to origin/${current_branch}..."
    git push origin "$current_branch"
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ ${submodule_name} pushed successfully${NC}"
    else
        echo -e "${RED}✗ Failed to push ${submodule_name}${NC}"
        cd - > /dev/null
        return 1
    fi
    
    cd - > /dev/null
    return 0
}

# Save current directory
PARENT_DIR=$(pwd)

# Push InverseModeling submodule
push_submodule "InverseModeling" "InverseModeling"
INVERSE_STATUS=$?

# Push Utilities submodule
push_submodule "Utilities" "Utilities"
UTILITIES_STATUS=$?

# Update parent repository
echo ""
echo -e "${YELLOW}Updating parent repository...${NC}"
cd "$PARENT_DIR"

if [ $INVERSE_STATUS -eq 0 ] || [ $UTILITIES_STATUS -eq 0 ]; then
    # At least one submodule was pushed, update parent
    if git diff-index --quiet HEAD -- InverseModeling Utilities; then
        echo "No submodule pointer updates needed"
    else
        git add InverseModeling Utilities
        git commit -m "Updated submodule pointers"
        git push origin main
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Parent repository updated${NC}"
        else
            echo -e "${RED}✗ Failed to push parent repository${NC}"
        fi
    fi
fi

echo ""
echo "====================================="
echo "Summary:"
if [ $INVERSE_STATUS -eq 0 ]; then
    echo -e "${GREEN}✓ InverseModeling: SUCCESS${NC}"
else
    echo -e "${RED}✗ InverseModeling: FAILED${NC}"
fi

if [ $UTILITIES_STATUS -eq 0 ]; then
    echo -e "${GREEN}✓ Utilities: SUCCESS${NC}"
else
    echo -e "${RED}✗ Utilities: FAILED${NC}"
fi
echo "====================================="
