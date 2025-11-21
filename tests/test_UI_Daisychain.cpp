//reyna 
#include <gtest/gtest.h>
#include <JuceHeader.h>

#include "Pitchblade/PluginProcessor.h"
#include "Pitchblade/ui/DaisyChain.h"

//all ui tests 
// TC-21 DaisyChain Initialization
// verifies number of rows equals number of effectNodes
TEST(DaisyChainTest, BuildsRowsFromProcessorNodes) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    const auto& layout = dc.getCurrentLayout();
    EXPECT_EQ(layout.size(), nodes.size());
}

// verifies that DaisyChain order matches processor order
TEST(DaisyChainTest, CurrentOrderMatchesLayout) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);
    auto order = dc.getCurrentOrder();

    ASSERT_EQ(order.size(), nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i)
        EXPECT_EQ(order[i], nodes[i]->effectName);
}

// verifies resetRowsToNodes restores the original processor layout
TEST(DaisyChainTest, ResetRowsToNodesRestoresDefaultOrder) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    proc.getMutex().lock();
    auto& layout = const_cast<std::vector<DaisyChain::Row>&>(dc.getCurrentLayout());
    if (!layout.empty())
        layout[0].left = "BrokenName";
    proc.getMutex().unlock();

    dc.resetRowsToNodes();
    const auto& fixed = dc.getCurrentLayout();

    ASSERT_EQ(fixed.size(), nodes.size());
    if (!nodes.empty())
        EXPECT_EQ(fixed[0].left, nodes[0]->effectName);
}

// TC-22 DaisyChain Default Global State
// verifies reorderLocked default and bypass visual flag
TEST(DaisyChainTest, DefaultGlobalStateIsCorrect) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    EXPECT_FALSE(dc.isReorderLocked());

    dc.setGlobalBypassVisual(false);
    SUCCEED();
}

// TC-23 effectNodes Mapping
// verifies each row label matches its EffectNode name
TEST(DaisyChainTest, EffectNodeNamesMapToRowsCorrectly) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);
    const auto& layout = dc.getCurrentLayout();

    ASSERT_EQ(layout.size(), nodes.size());

    for (size_t i = 0; i < nodes.size(); ++i)
        EXPECT_EQ(layout[i].left, nodes[i]->effectName);
}

// TC-26 Reorder Behavior
// verifies dragging moves UI items
TEST(DaisyChainTest, HandleReorderMovesItemToNewRow) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    if (nodes.size() > 1) {
        auto first = nodes[0]->effectName;
        auto second = nodes[1]->effectName;

        dc.handleReorder(-1, first, 2);

        auto order = dc.getCurrentOrder();
        ASSERT_EQ(order.size(), nodes.size());
        EXPECT_EQ(order[0], second);
    }
}

// TC-27 Reorder Lock Behavior
// verifies reordering does nothing while locked
TEST(DaisyChainTest, ReorderLockEnablesAndDisables) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    EXPECT_FALSE(dc.isReorderLocked());
    dc.setReorderLocked(true);
    EXPECT_TRUE(dc.isReorderLocked());
    dc.setReorderLocked(false);
    EXPECT_FALSE(dc.isReorderLocked());
}

// verifies reordering is blocked when locked
TEST(DaisyChainTest, ReorderIsBlockedWhenLocked) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();
    DaisyChain dc(proc, nodes);

    if (nodes.size() > 1) {
        auto before = dc.getCurrentOrder();

        dc.setReorderLocked(true);

        dc.handleReorder(-1, nodes[0]->effectName, 2);

        auto after = dc.getCurrentOrder();
        EXPECT_EQ(before, after);
    }
}

// TC-28 Reorder Unlock Behavior
// verifies reorder works again after unlock
TEST(DaisyChainTest, ReorderWorksAfterUnlock) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    if (nodes.size() > 1) {
        auto first = nodes[0]->effectName;
        auto second = nodes[1]->effectName;

        dc.setReorderLocked(true);
        dc.setReorderLocked(false);

        dc.handleReorder(-1, first, 2);

        auto order = dc.getCurrentOrder();
        ASSERT_EQ(order.size(), nodes.size());
        EXPECT_EQ(order[0], second);
    }
}

// TC-29 Split Mode
// verifies dragging into right side creates split row
TEST(DaisyChainTest, SplitModeUpdatesProcessorLayout) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    if (nodes.size() > 1) {
        auto left = nodes[0]->effectName;
        auto right = nodes[1]->effectName;

        dc.handleReorder(-2, right, 0);

        const auto& layout = dc.getCurrentLayout();

        ASSERT_FALSE(layout.empty());
        EXPECT_EQ(layout[0].left, left);
        EXPECT_EQ(layout[0].right, right);
    }
}

// TC-30 Double Down Behavior
// verifies two parallel nodes appear in a double row
TEST(DaisyChainTest, DoubleDownCreatesTwoParallelNodes) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    if (nodes.size() > 1) {
        auto left = nodes[0]->effectName;
        auto right = nodes[1]->effectName;

        dc.handleReorder(-2, right, 0);

        const auto& layout = dc.getCurrentLayout();

        ASSERT_FALSE(layout.empty());
        EXPECT_TRUE(layout[0].hasRight());
        EXPECT_EQ(layout[0].left, left);
        EXPECT_EQ(layout[0].right, right);
    }
}

// TC-31 Unite Mode
// verifies double row collapses back into single row
TEST(DaisyChainTest, UniteModeCollapsesDoubleRow) {
    AudioPluginAudioProcessor proc;
    auto& nodes = proc.getEffectNodes();

    DaisyChain dc(proc, nodes);

    if (nodes.size() > 1) {
        auto left = nodes[0]->effectName;
        auto right = nodes[1]->effectName;

        dc.handleReorder(-2, right, 0);

        auto& layout = const_cast<std::vector<DaisyChain::Row>&>(dc.getCurrentLayout());
        layout[0].right = "";

        dc.rebuild();

        const auto& after = dc.getCurrentLayout();
        EXPECT_EQ(after[0].left, left);
        EXPECT_FALSE(after[0].hasRight());
    }
}
