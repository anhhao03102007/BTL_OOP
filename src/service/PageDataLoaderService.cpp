        #include "services/PageDataLoaderService.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace Banking {
namespace {

json failure(const std::string& page, const std::string& message) {
    return {
        {"success", false},
        {"page", page},
        {"message", message},
        {"data", json::object()}
    };
}

std::string loanType(const LoanModel& loan) {
    return loan.getCollateralInfo().empty() ? "Vay tiêu dùng" : "Vay thế chấp";
}

int loanStage(const std::string& status) {
    if (status == "APPROVED") return 3;
    if (status == "DISBURSED" || status == "CLOSED") return 4;
    if (status == "REJECTED") return -1;
    return 1; // PENDING / đang thẩm định
}

json missingLoanDocuments() {
    return json::array({
        {{"id", "cccd"}, {"icon", "shield"}, {"name", "CCCD gắn chip (2 mặt)"},
         {"status", "MISSING"}, {"hint", "Chưa có tệp"}, {"file", ""}, {"mime", ""}, {"size", ""}, {"at", ""}, {"url", ""}},
        {{"id", "income"}, {"icon", "wallet"}, {"name", "Sao kê lương / HĐLĐ 3 tháng gần nhất"},
         {"status", "MISSING"}, {"hint", "Chưa có tệp"}, {"file", ""}, {"mime", ""}, {"size", ""}, {"at", ""}, {"url", ""}},
        {{"id", "collateral"}, {"icon", "file"}, {"name", "Giấy tờ tài sản đảm bảo"},
         {"status", "MISSING"}, {"hint", "Chưa có tệp"}, {"file", ""}, {"mime", ""}, {"size", ""}, {"at", ""}, {"url", ""}},
        {{"id", "address"}, {"icon", "home"}, {"name", "Xác nhận nơi cư trú"},
         {"status", "MISSING"}, {"hint", "Chưa có tệp"}, {"file", ""}, {"mime", ""}, {"size", ""}, {"at", ""}, {"url", ""}}
    });
}

} // namespace

PageDataLoaderService::PageDataLoaderService(UserService& userSvc, AccountService& accountSvc, LoanService& loanSvc, SavingsService& savingsSvc)
    : m_userService(userSvc), m_accountService(accountSvc), m_loanService(loanSvc), m_savingsService(savingsSvc) {}

json PageDataLoaderService::loadDataForPage(const std::string& page, const std::string& sessionToken) {
    try {
        const UserProfileDTO profile = m_userService.getProfile(sessionToken);
        if (profile.user_id.empty()) {
            return failure(page, "Phiên đăng nhập đã hết hạn hoặc không hợp lệ.");
        }
        if (profile.role != "CUSTOMER") {
            return failure(page, "Nạp dữ liệu cho giao diện nhân viên chưa được triển khai.");
        }

        json data;
        if (page == "home") {
            data = loadHomePageDataForCUS(profile, sessionToken);
        } else if (page == "accounts") {
            data = loadAccountsPageData(profile);
        } else if (page == "transactions") {
            data = loadTransactionsPageData(profile, sessionToken);
        } else if (page == "user-settings") {
            data = loadSettingPageData(profile);
        } else if (page == "savings") {
            data = loadSavingsPageData(profile, sessionToken);
        } else if (page == "services" || page == "transfer" || page == "topup" || page == "payment") {
            data = loadServicePageData(profile);
        } else if (page == "apply-loan" || page == "loan" || page == "loan-pay") {
            data = loadLoanPageData(profile, sessionToken);
        } else {
            return failure(page, "Trang này chưa có nguồn dữ liệu backend.");
        }

        return {
            {"success", true},
            {"page", page},
            {"message", "Nạp dữ liệu thành công."},
            {"data", data}
        };
    } catch (const std::exception& error) {
        return failure(page, error.what());
    } catch (...) {
        return failure(page, "Không thể nạp dữ liệu từ cơ sở dữ liệu.");
    }
}

json PageDataLoaderService::loadHomePageDataForCUS(const UserProfileDTO& profile, const std::string& sessionToken) {
    return loadTransactionsPageData(profile, sessionToken);
}

json PageDataLoaderService::loadAccountsPageData(const UserProfileDTO& profile) {
    const GetAccountsResult result = m_accountService.getAccounts(profile.user_id, profile.user_id);
    if (!result.success) {
        throw std::runtime_error(result.message.empty() ? "Không thể tải danh sách tài khoản." : result.message);
    }
    return {
        {"profile", profile},
        {"accounts", result.accounts}
    };
}

json PageDataLoaderService::loadServicePageData(const UserProfileDTO& profile) {
    return loadAccountsPageData(profile);
}

json PageDataLoaderService::loadSettingPageData(const UserProfileDTO& profile) {
    return {{"profile", profile}};
}

json PageDataLoaderService::loadSavingsPageData(const UserProfileDTO& profile, const std::string& sessionToken) {
    json data = loadAccountsPageData(profile);
    GetSavingsListRequest request;
    request.session_token = sessionToken;
    request.target_user_id = profile.user_id;
    request.status_filter = "ALL";
    const GetSavingsListResult result = m_savingsService.getSavingsList(request);
    if (!result.success) {
        throw std::runtime_error(result.message.empty() ? "Không thể tải danh sách sổ tiết kiệm." : result.message);
    }
    data["savings"] = result.items;
    return data;
}

json PageDataLoaderService::loadTransactionsPageData(const UserProfileDTO& profile, const std::string& sessionToken) {
    const GetAccountsResult accounts = m_accountService.getAccounts(profile.user_id, profile.user_id);
    if (!accounts.success) {
        throw std::runtime_error(accounts.message.empty() ? "Không thể tải danh sách tài khoản." : accounts.message);
    }

    std::string accountId = profile.primary_account_id;
    const auto primary = std::find_if(accounts.accounts.begin(), accounts.accounts.end(),
        [&](const AccountDTO& account) { return account.account_id == accountId; });
    if (primary == accounts.accounts.end()) {
        accountId = accounts.accounts.empty() ? "" : accounts.accounts.front().account_id;
    }

    json transactions = json::array();
    if (!accountId.empty()) {
        const TransactionHistoryResult history =
            m_accountService.getTransactionHistory(profile.user_id, accountId, 50, 1);
        if (!history.success) {
            throw std::runtime_error(history.message.empty() ? "Không thể tải lịch sử giao dịch." : history.message);
        }
        transactions = history.items;
    }

    return {
        {"profile", profile},
        {"accounts", accounts.accounts},
        {"transactions", transactions}
    };
}

json PageDataLoaderService::loadLoanPageData(const UserProfileDTO& profile, const std::string& sessionToken) {
    json data = loadAccountsPageData(profile);
    data["loanApps"] = json::array();
    data["activeLoans"] = json::array();
    data["loanTerms"] = m_loanService.getAvailableTerms();

    const std::vector<LoanModel> loans = m_loanService.getLoansForSession(sessionToken);
    for (const LoanModel& loan : loans) {
        const int stage = loanStage(loan.getStatus());
        const std::string method = loan.getCalcMethod() == "FLAT" ? "FIXED" : loan.getCalcMethod();
        data["loanApps"].push_back({
            {"id", loan.getLoanId()},
            {"who", profile.fullname},
            {"phone", profile.phone},
            {"type", loanType(loan)},
            {"amt", loan.getOriginalAmountVnd()},
            {"term", loan.getTermMonths()},
            {"rate", loan.getAnnualInterestRate()},
            {"method", method},
            {"purpose", "—"},
            {"collateral", loan.getCollateralInfo()},
            {"income", ""},
            {"stage", stage},
            {"needInfo", false},
            {"resubmitted", false},
            {"result", nullptr},
            {"docs", missingLoanDocuments()}
        });

        if (loan.getStatus() == "DISBURSED" && loan.getRemainingDebtVnd() > 0) {
            const int64_t monthlyPrincipal = std::max<int64_t>(1, loan.calculateMonthlyPrincipal());
            const int left = static_cast<int>(std::min<int64_t>(
                loan.getTermMonths(),
                (loan.getRemainingDebtVnd() + monthlyPrincipal - 1) / monthlyPrincipal));
            data["activeLoans"].push_back({
                {"id", loan.getLoanId()},
                {"type", loanType(loan)},
                {"principal", loan.getOriginalAmountVnd()},
                {"outstanding", loan.getRemainingDebtVnd()},
                {"rate", loan.getAnnualInterestRate()},
                {"total", loan.getTermMonths()},
                {"left", left},
                {"pay", loan.calculateMonthlyInstallment()},
                {"due", ""}
            });
        }
    }
    return data;
}

} // namespace Banking
