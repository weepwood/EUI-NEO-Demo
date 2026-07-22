#include "eui_neo.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <functional>
#include <string>

namespace app {
namespace {

struct DemoState {
    int selectedPage = 0;
    int selectedDensity = 1;
    bool darkMode = true;
    bool compactCards = false;
    int runCount = 12;
    float scrollOffset = 0.0f;
    std::string workspaceName = "Atlas Workspace";
    std::string searchText;
    std::string feedback = "System ready";
    std::array<bool, 4> tasks{false, true, false, false};
    eui::Signal<bool> notifications{true};
    eui::Signal<bool> autoDeploy{false};
    eui::Signal<float> capacity{0.68f};
};

DemoState state;

components::theme::ThemeColorTokens themeColors() {
    auto tokens = state.darkMode ? components::theme::dark() : components::theme::light();
    tokens.primary = {0.31f, 0.53f, 0.96f, 1.0f};
    return tokens;
}

components::theme::PageVisualTokens pageVisuals() {
    return components::theme::pageVisuals(themeColors());
}

eui::Transition motion() {
    return eui::Transition::make(0.24f, eui::Ease::OutCubic);
}

eui::Color transparent() {
    return {0.0f, 0.0f, 0.0f, 0.0f};
}

eui::Color mutedText() {
    return pageVisuals().subtitleColor;
}

eui::Color bodyText() {
    return pageVisuals().bodyColor;
}

eui::Color borderColor(float opacity = 1.0f) {
    return components::theme::withOpacity(themeColors().border, opacity);
}

eui::Color shadowColor() {
    return state.darkMode
        ? eui::Color{0.0f, 0.0f, 0.0f, 0.28f}
        : eui::Color{0.10f, 0.14f, 0.22f, 0.12f};
}

std::string percentText(float value) {
    char buffer[16] = {};
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%d%%",
        static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 100.0f + 0.5f));
    return buffer;
}

void panel(eui::Ui& ui,
           const std::string& id,
           float width,
           float height,
           const std::function<void()>& content) {
    ui.stack(id)
        .size(width, height)
        .content([&] {
            ui.rect(id + ".background")
                .size(width, height)
                .color(themeColors().surface)
                .radius(state.compactCards ? 13.0f : 19.0f)
                .border(1.0f, borderColor(0.78f))
                .shadow(24.0f, 0.0f, 8.0f, shadowColor())
                .transition(motion())
                .build();

            ui.column(id + ".content")
                .size(width, height)
                .padding(state.compactCards ? 15.0f : 20.0f)
                .gap(state.compactCards ? 8.0f : 12.0f)
                .content(content)
                .build();
        })
        .build();
}

void metricCard(eui::Ui& ui,
                const std::string& id,
                const std::string& label,
                const std::string& value,
                const std::string& note,
                float progressValue,
                float width) {
    const float height = state.compactCards ? 132.0f : 154.0f;
    panel(ui, id, width, height, [&] {
        ui.text(id + ".label")
            .size(width - 40.0f, 22.0f)
            .text(label)
            .fontSize(14.0f)
            .lineHeight(20.0f)
            .color(mutedText())
            .build();

        ui.text(id + ".value")
            .size(width - 40.0f, 39.0f)
            .text(value)
            .fontSize(state.compactCards ? 27.0f : 32.0f)
            .lineHeight(38.0f)
            .fontWeight(760)
            .color(pageVisuals().titleColor)
            .build();

        components::progress(ui, id + ".progress")
            .theme(themeColors())
            .size(width - 40.0f, 9.0f)
            .value(progressValue)
            .transition(motion())
            .build();

        ui.text(id + ".note")
            .size(width - 40.0f, 20.0f)
            .text(note)
            .fontSize(13.0f)
            .lineHeight(18.0f)
            .color(bodyText())
            .build();
    });
}

void taskRow(eui::Ui& ui,
             const std::string& id,
             int index,
             const std::string& title,
             const std::string& owner,
             float width) {
    const bool done = state.tasks[static_cast<std::size_t>(index)];
    const float actionWidth = 96.0f;
    const float copyWidth = std::max(120.0f, width - actionWidth - 10.0f);

    ui.row(id)
        .size(width, 52.0f)
        .gap(10.0f)
        .alignItems(eui::Align::CENTER)
        .content([&] {
            ui.column(id + ".copy")
                .size(copyWidth, 52.0f)
                .gap(2.0f)
                .justifyContent(eui::Align::CENTER)
                .content([&] {
                    ui.text(id + ".title")
                        .size(copyWidth, 23.0f)
                        .text(title)
                        .fontSize(15.0f)
                        .lineHeight(21.0f)
                        .fontWeight(680)
                        .color(pageVisuals().titleColor)
                        .build();

                    ui.text(id + ".owner")
                        .size(copyWidth, 19.0f)
                        .text(owner)
                        .fontSize(12.0f)
                        .lineHeight(17.0f)
                        .color(mutedText())
                        .build();
                })
                .build();

            components::button(ui, id + ".action")
                .theme(themeColors(), false)
                .size(actionWidth, 36.0f)
                .text(done ? "Reopen" : "Complete")
                .textColor(done ? mutedText() : themeColors().primary)
                .colors(
                    transparent(),
                    components::theme::withAlpha(themeColors().primary, 0.10f),
                    components::theme::withAlpha(themeColors().primary, 0.18f))
                .border(
                    1.0f,
                    done ? borderColor(0.62f)
                         : components::theme::withAlpha(themeColors().primary, 0.58f))
                .shadow(0.0f, 0.0f, 0.0f, transparent())
                .radius(10.0f)
                .transition(motion())
                .onClick([index] {
                    auto& value = state.tasks[static_cast<std::size_t>(index)];
                    value = !value;
                    state.feedback = value ? "Task completed" : "Task reopened";
                })
                .build();
        })
        .build();
}

void composeOverview(eui::Ui& ui, float width) {
    const float usableWidth = std::max(280.0f, width - 56.0f);
    const int metricColumns = usableWidth >= 980.0f ? 4 : (usableWidth >= 560.0f ? 2 : 1);
    const float metricGap = 16.0f;
    const float metricWidth = std::max(
        210.0f,
        (usableWidth - metricGap * static_cast<float>(metricColumns - 1)) /
            static_cast<float>(metricColumns));
    const bool splitPanels = usableWidth >= 900.0f;
    const float panelGap = 18.0f;
    const float detailWidth = splitPanels ? (usableWidth - panelGap) * 0.5f : usableWidth;

    ui.column("overview.page")
        .width(usableWidth)
        .height(eui::SizeValue::wrapContent())
        .margin(28.0f)
        .gap(18.0f)
        .content([&] {
            panel(ui, "overview.hero", usableWidth, 184.0f, [&] {
                ui.text("overview.hero.eyebrow")
                    .size(usableWidth - 40.0f, 22.0f)
                    .text("EUI-NEO DESKTOP EXPERIENCE")
                    .fontSize(13.0f)
                    .lineHeight(18.0f)
                    .fontWeight(760)
                    .color(themeColors().primary)
                    .build();

                ui.text("overview.hero.title")
                    .size(usableWidth - 40.0f, 42.0f)
                    .text("Command Center")
                    .fontSize(34.0f)
                    .lineHeight(40.0f)
                    .fontWeight(780)
                    .color(pageVisuals().titleColor)
                    .build();

                ui.text("overview.hero.subtitle")
                    .size(usableWidth - 40.0f, 28.0f)
                    .text("A native C++ dashboard demonstrating layout, state, controls and animation.")
                    .fontSize(16.0f)
                    .lineHeight(23.0f)
                    .color(bodyText())
                    .build();

                ui.row("overview.hero.controls")
                    .size(usableWidth - 40.0f, 44.0f)
                    .gap(14.0f)
                    .alignItems(eui::Align::CENTER)
                    .content([&] {
                        components::slider(ui, "overview.capacity.slider")
                            .theme(themeColors())
                            .size(std::max(120.0f, usableWidth - 244.0f), 32.0f)
                            .bind(state.capacity)
                            .transition(motion())
                            .build();

                        ui.text("overview.capacity.value")
                            .size(58.0f, 28.0f)
                            .text(percentText(state.capacity.get()))
                            .fontSize(15.0f)
                            .lineHeight(22.0f)
                            .fontWeight(700)
                            .color(pageVisuals().titleColor)
                            .horizontalAlign(eui::HorizontalAlign::Center)
                            .build();

                        components::button(ui, "overview.launch")
                            .theme(themeColors())
                            .size(132.0f, 40.0f)
                            .icon(0xF04B)
                            .text("Run demo")
                            .transition(motion())
                            .onClick([] {
                                state.runCount += 1;
                                state.feedback = "Demo workflow started";
                            })
                            .build();
                    })
                    .build();
            });

            ui.flow("overview.metrics")
                .width(usableWidth)
                .height(eui::SizeValue::wrapContent())
                .gap(metricGap)
                .lineGap(metricGap)
                .content([&] {
                    metricCard(ui, "metric.workflows", "Active workflows", "24", "+6 this week", 0.74f, metricWidth);
                    metricCard(ui, "metric.success", "Build success", "98.4%", "Stable across 3 platforms", 0.984f, metricWidth);
                    metricCard(ui, "metric.latency", "P95 render time", "5.8 ms", "Inside the 8 ms target", 0.72f, metricWidth);
                    metricCard(ui, "metric.tasks", "Open tasks", "7", "3 items are high priority", 0.46f, metricWidth);
                })
                .build();

            ui.flow("overview.details")
                .width(usableWidth)
                .height(eui::SizeValue::wrapContent())
                .gap(panelGap)
                .lineGap(panelGap)
                .content([&] {
                    panel(ui, "overview.health", detailWidth, 342.0f, [&] {
                        ui.text("overview.health.title")
                            .size(detailWidth - 40.0f, 30.0f)
                            .text("Project health")
                            .fontSize(22.0f)
                            .lineHeight(28.0f)
                            .fontWeight(740)
                            .color(pageVisuals().titleColor)
                            .build();

                        const std::array<std::string, 4> labels{
                            "UI composition", "Rendering", "Interaction", "Packaging"};
                        const std::array<float, 4> values{0.92f, 0.84f, 0.76f, 0.68f};

                        for (std::size_t i = 0; i < labels.size(); ++i) {
                            const std::string id = "overview.health.item." + std::to_string(i);
                            ui.row(id + ".header")
                                .size(detailWidth - 40.0f, 22.0f)
                                .content([&, i, id] {
                                    ui.text(id + ".label")
                                        .size(detailWidth - 112.0f, 22.0f)
                                        .text(labels[i])
                                        .fontSize(14.0f)
                                        .lineHeight(20.0f)
                                        .color(bodyText())
                                        .build();
                                    ui.text(id + ".value")
                                        .size(72.0f, 22.0f)
                                        .text(percentText(values[i]))
                                        .fontSize(14.0f)
                                        .lineHeight(20.0f)
                                        .fontWeight(700)
                                        .color(pageVisuals().titleColor)
                                        .horizontalAlign(eui::HorizontalAlign::Right)
                                        .build();
                                })
                                .build();

                            components::progress(ui, id + ".progress")
                                .theme(themeColors())
                                .size(detailWidth - 40.0f, 8.0f)
                                .value(values[i])
                                .transition(motion())
                                .build();
                        }
                    });

                    panel(ui, "overview.tasks", detailWidth, 342.0f, [&] {
                        ui.text("overview.tasks.title")
                            .size(detailWidth - 40.0f, 30.0f)
                            .text("Next actions")
                            .fontSize(22.0f)
                            .lineHeight(28.0f)
                            .fontWeight(740)
                            .color(pageVisuals().titleColor)
                            .build();

                        taskRow(ui, "task.design", 0, "Review component spacing", "Design system", detailWidth - 40.0f);
                        taskRow(ui, "task.windows", 1, "Verify Windows package", "Release pipeline", detailWidth - 40.0f);
                        taskRow(ui, "task.keyboard", 2, "Add keyboard navigation", "Accessibility", detailWidth - 40.0f);
                        taskRow(ui, "task.metrics", 3, "Capture render metrics", "Performance", detailWidth - 40.0f);
                    });
                })
                .build();
        })
        .build();
}

void composeControls(eui::Ui& ui, float width) {
    const float usableWidth = std::max(280.0f, width - 56.0f);
    const bool split = usableWidth >= 860.0f;
    const float gap = 18.0f;
    const float cardWidth = split ? (usableWidth - gap) * 0.5f : usableWidth;

    ui.column("controls.page")
        .width(usableWidth)
        .height(eui::SizeValue::wrapContent())
        .margin(28.0f)
        .gap(18.0f)
        .content([&] {
            ui.text("controls.title")
                .size(usableWidth, 42.0f)
                .text("Interactive controls")
                .fontSize(32.0f)
                .lineHeight(40.0f)
                .fontWeight(780)
                .color(pageVisuals().titleColor)
                .build();

            ui.text("controls.subtitle")
                .size(usableWidth, 26.0f)
                .text("All widgets below are backed by local application state.")
                .fontSize(16.0f)
                .lineHeight(23.0f)
                .color(mutedText())
                .build();

            ui.flow("controls.cards")
                .width(usableWidth)
                .height(eui::SizeValue::wrapContent())
                .gap(gap)
                .lineGap(gap)
                .content([&] {
                    panel(ui, "controls.workspace", cardWidth, 292.0f, [&] {
                        ui.text("controls.workspace.title")
                            .size(cardWidth - 40.0f, 30.0f)
                            .text("Workspace")
                            .fontSize(22.0f)
                            .lineHeight(28.0f)
                            .fontWeight(740)
                            .color(pageVisuals().titleColor)
                            .build();

                        components::input(ui, "controls.workspace.name")
                            .theme(themeColors())
                            .size(cardWidth - 40.0f, 44.0f)
                            .value(state.workspaceName)
                            .placeholder("Workspace name")
                            .onChange([](const std::string& value) {
                                state.workspaceName = value;
                                state.feedback = "Workspace name updated";
                            })
                            .build();

                        components::input(ui, "controls.workspace.search")
                            .theme(themeColors())
                            .size(cardWidth - 40.0f, 44.0f)
                            .value(state.searchText)
                            .placeholder("Search commands")
                            .onChange([](const std::string& value) {
                                state.searchText = value;
                                state.feedback = value.empty() ? "Search cleared" : "Search updated";
                            })
                            .build();

                        components::segmented(ui, "controls.workspace.density")
                            .theme(themeColors())
                            .size(cardWidth - 40.0f, 38.0f)
                            .items({"Comfortable", "Balanced", "Compact"})
                            .selected(state.selectedDensity)
                            .transition(motion())
                            .onChange([](int index) {
                                state.selectedDensity = index;
                                state.compactCards = index == 2;
                                state.feedback = "Density changed";
                            })
                            .build();
                    });

                    panel(ui, "controls.automation", cardWidth, 292.0f, [&] {
                        ui.text("controls.automation.title")
                            .size(cardWidth - 40.0f, 30.0f)
                            .text("Automation")
                            .fontSize(22.0f)
                            .lineHeight(28.0f)
                            .fontWeight(740)
                            .color(pageVisuals().titleColor)
                            .build();

                        components::toggleSwitch(ui, "controls.notifications")
                            .theme(themeColors())
                            .size(cardWidth - 40.0f, 34.0f)
                            .bind(state.notifications)
                            .text("Desktop notifications")
                            .transition(motion())
                            .build();

                        components::toggleSwitch(ui, "controls.deploy")
                            .theme(themeColors())
                            .size(cardWidth - 40.0f, 34.0f)
                            .bind(state.autoDeploy)
                            .text("Automatic deployment")
                            .transition(motion())
                            .build();

                        ui.text("controls.capacity.label")
                            .size(cardWidth - 40.0f, 22.0f)
                            .text("Resource capacity: " + percentText(state.capacity.get()))
                            .fontSize(14.0f)
                            .lineHeight(20.0f)
                            .color(bodyText())
                            .build();

                        components::slider(ui, "controls.capacity")
                            .theme(themeColors())
                            .size(cardWidth - 40.0f, 32.0f)
                            .bind(state.capacity)
                            .transition(motion())
                            .build();

                        components::button(ui, "controls.reset")
                            .theme(themeColors(), false)
                            .size(cardWidth - 40.0f, 42.0f)
                            .icon(0xF2EA)
                            .text("Reset demo state")
                            .transition(motion())
                            .onClick([] {
                                state.capacity.set(0.68f);
                                state.notifications.set(true);
                                state.autoDeploy.set(false);
                                state.selectedDensity = 1;
                                state.compactCards = false;
                                state.feedback = "Demo state reset";
                            })
                            .build();
                    });
                })
                .build();

            panel(ui, "controls.feedback", usableWidth, 104.0f, [&] {
                ui.text("controls.feedback.label")
                    .size(usableWidth - 40.0f, 22.0f)
                    .text("Latest interaction")
                    .fontSize(13.0f)
                    .lineHeight(18.0f)
                    .fontWeight(720)
                    .color(themeColors().primary)
                    .build();

                ui.text("controls.feedback.value")
                    .size(usableWidth - 40.0f, 30.0f)
                    .text(state.feedback)
                    .fontSize(18.0f)
                    .lineHeight(26.0f)
                    .color(pageVisuals().titleColor)
                    .build();
            });
        })
        .build();
}

void composeAbout(eui::Ui& ui, float width) {
    const float usableWidth = std::max(280.0f, width - 56.0f);

    ui.column("about.page")
        .width(usableWidth)
        .height(eui::SizeValue::wrapContent())
        .margin(28.0f)
        .gap(18.0f)
        .content([&] {
            panel(ui, "about.hero", usableWidth, 330.0f, [&] {
                ui.text("about.hero.eyebrow")
                    .size(usableWidth - 40.0f, 24.0f)
                    .text("ABOUT THIS DEMO")
                    .fontSize(13.0f)
                    .lineHeight(18.0f)
                    .fontWeight(760)
                    .color(themeColors().primary)
                    .build();

                ui.text("about.hero.title")
                    .size(usableWidth - 40.0f, 46.0f)
                    .text("EUI-NEO Command Center")
                    .fontSize(32.0f)
                    .lineHeight(42.0f)
                    .fontWeight(780)
                    .color(pageVisuals().titleColor)
                    .build();

                ui.text("about.hero.body")
                    .size(usableWidth - 40.0f, 92.0f)
                    .text("This application is a standalone C++17 example built with EUI-NEO. It demonstrates responsive layout, retained state, animation, input controls, progress indicators, navigation and Windows packaging through GitHub Actions.")
                    .fontSize(16.0f)
                    .lineHeight(24.0f)
                    .wrap(true)
                    .color(bodyText())
                    .build();

                ui.text("about.hero.stack")
                    .size(usableWidth - 40.0f, 26.0f)
                    .text("C++17  /  CMake  /  GLFW  /  OpenGL")
                    .fontSize(15.0f)
                    .lineHeight(22.0f)
                    .fontWeight(700)
                    .color(themeColors().primary)
                    .build();

                components::button(ui, "about.hero.action")
                    .theme(themeColors())
                    .size(190.0f, 42.0f)
                    .icon(0xF09B)
                    .text("EUI-NEO project")
                    .transition(motion())
                    .onClick([] {
                        state.feedback = "Repository action selected";
                    })
                    .build();
            });
        })
        .build();
}

void composePage(eui::Ui& ui, float width, float height) {
    components::scrollView(ui, "content.scroll")
        .theme(themeColors())
        .size(width, height)
        .offset(state.scrollOffset)
        .step(52.0f)
        .contentKey(std::to_string(state.selectedPage) + (state.compactCards ? ".compact" : ".regular"))
        .onChange([](float value) {
            state.scrollOffset = value;
        })
        .content([&](eui::Ui& contentUi, float contentWidth, float) {
            if (state.selectedPage == 1) {
                composeControls(contentUi, contentWidth);
            } else if (state.selectedPage == 2) {
                composeAbout(contentUi, contentWidth);
            } else {
                composeOverview(contentUi, contentWidth);
            }
        })
        .build();
}

} // namespace

const DslAppConfig& dslAppConfig() {
    static const DslAppConfig config = DslAppConfig{}
        .title("EUI-NEO Demo")
        .pageId("eui_neo_demo")
        .clearColor({0.07f, 0.08f, 0.10f, 1.0f})
        .windowSize(1440, 920)
        .fps(90.0);
    return config;
}

void compose(eui::Ui& ui, const eui::Screen& screen) {
    const bool compactNavigation = screen.width < 920.0f;
    const float navigationWidth = compactNavigation ? 96.0f : 250.0f;
    const float contentWidth = std::max(0.0f, screen.width - navigationWidth);

    ui.row("root")
        .size(screen.width, screen.height)
        .content([&] {
            components::navbar(ui, "navigation")
                .theme(themeColors())
                .size(navigationWidth, screen.height)
                .compact(compactNavigation)
                .brand("EUI-NEO Demo", 0xF5FD)
                .subtitle("Native command center")
                .selected(state.selectedPage)
                .items({
                    {"overview", "Overview", 0xF201, 0},
                    {"controls", "Controls", 0xF1DE, 1},
                    {"about", "About", 0xF05A, 2},
                })
                .footer(
                    state.darkMode ? "Light mode" : "Dark mode",
                    state.darkMode ? 0xF185 : 0xF186,
                    [] {
                        state.darkMode = !state.darkMode;
                        state.feedback = state.darkMode ? "Dark mode enabled" : "Light mode enabled";
                    })
                .transition(motion())
                .onChange([](int page) {
                    state.selectedPage = page;
                    state.scrollOffset = 0.0f;
                    state.feedback = "Page changed";
                })
                .build();

            ui.stack("content")
                .size(contentWidth, screen.height)
                .content([&] {
                    ui.rect("content.background")
                        .size(contentWidth, screen.height)
                        .color(themeColors().background)
                        .transition(motion())
                        .build();

                    composePage(ui, contentWidth, screen.height);
                })
                .build();
        })
        .build();
}

} // namespace app
